-- Replay a chess game from PGN (Portable Game Notation).
-- Usage: ./artc ./examples/chess_pgn.lua [-- ./examples/sample.pgn]
-- Pass `--` then a path to load that file; otherwise uses default path or EMBEDDED_PGN fallback.

window(800, 800)
bg("#1e1812")
fps(45)
seed(1)

--------------------------------------------------------------------------------
-- Config
--------------------------------------------------------------------------------

local PGN_PATH = (type(arg) == "table" and arg[1] ~= nil and arg[1] ~= "") and arg[1] or "./examples/sample.pgn"
local SECONDS_PER_HALFMOVE = 0.78
-- After the last half-move animation finishes, wait this many seconds then call quit() (clean exit for ffmpeg etc.). nil = keep running.
local QUIT_AFTER_DONE_S = 1
local quit_after_deadline_ms = nil

local EMBEDDED_PGN = [[
[Event "Embedded fallback"]
1. e4 e5 2. Nf3 Nc6 3. Bb5 a6 4. Ba4 Nf6 5. O-O Be7 *
]]

--------------------------------------------------------------------------------
-- Layout (same as chess.lua)
--------------------------------------------------------------------------------

local CELL = 72
local PIECE_SCALE = 0.88
local BOARD = 8 * CELL
local ORIG_X = (800 - BOARD) / 2
-- Set in setup() after measuring header block (must stay above the board).
local BOARD_ORIG_Y = (800 - BOARD) / 2 + 24

local HEADER_PAD_TOP = 18
local HEADER_LINE_STEP = 21
local HEADER_FONT_SIZE = 16
local HEADER_GAP_BEFORE_BOARD = 22
local TITLE_BELOW_HEADERS = 10
local TITLE_BAND = 24 -- reserved height for status line before board frame

local BASE_LIGHT = { r = 240, g = 217, b = 181 }
local BASE_DARK = { r = 181, g = 133, b = 99 }

local PIECE_DIR = "./assets/pieces/"
local squares = {}
local piece_entity = {} -- key "f,r" -> image entity id
local title_id
local title_y = 0
local last_status_text
local header_ids = {}

local game -- current position (updated after each completed half-move)
local move_index = 0 -- half-moves completed (0 = start)
local anim = nil -- { kind="one"|"castle", ... }

--------------------------------------------------------------------------------
-- PGN text → SAN tokens
--------------------------------------------------------------------------------

local function strip_bracket_headers(s)
    return s:gsub("%b[]", " ")
end

local function strip_brace_comments(s)
    while true do
        local a, b = s:find("%b{}")
        if not a then
            break
        end
        s = s:sub(1, a - 1) .. " " .. s:sub(b + 1)
    end
    return s
end

local function strip_parentheses(s)
    while true do
        local i = s:find("%(")
        if not i then
            break
        end
        local depth = 0
        local j
        for k = i, #s do
            local c = s:sub(k, k)
            if c == "(" then
                depth = depth + 1
            elseif c == ")" then
                depth = depth - 1
                if depth == 0 then
                    j = k
                    break
                end
            end
        end
        if not j then
            break
        end
        s = s:sub(1, i - 1) .. " " .. s:sub(j + 1)
    end
    return s
end

local function tokenize_movetext(body)
    body = strip_bracket_headers(body)
    body = strip_brace_comments(body)
    body = strip_parentheses(body)
    body = body:gsub("%$%d+", " ")
    local tokens = {}
    for w in body:gmatch("%S+") do
        if
            not w:find("^%d+%.%d*%.%.%.?$")
            and not w:find("^%d+%.$")
            and w ~= "*"
            and w ~= "1-0"
            and w ~= "0-1"
            and w ~= "1/2-1/2"
            and w ~= "1/2"
            and not w:find("^%d+%-%d+$")
        then
            table.insert(tokens, w)
        end
    end
    return tokens
end

local function load_pgn_text()
    if PGN_PATH and type(read_file) == "function" and file_exists(PGN_PATH) then
        local t = read_file(PGN_PATH)
        if type(t) == "string" and t ~= "" then
            return t
        end
    end
    return EMBEDDED_PGN
end

--- [Key "Value"] lines (standard PGN seven-tag roster and friends)
local function parse_pgn_headers(pgn)
    local h = {}
    if not pgn or pgn == "" then
        return h
    end
    for line in pgn:gmatch("[^\r\n]+") do
        local key, val = line:match("^%[([^%s%]]+)%s+\"(.-)\"%s*%]")
        if key and val then
            h[key] = val
        end
    end
    return h
end

local function header_display_lines(h)
    local lines = {}
    if h.Event and h.Event ~= "" and h.Event ~= "?" then
        table.insert(lines, h.Event)
    end
    local wn = h.White
    local bn = h.Black
    if (wn and wn ~= "" and wn ~= "?") or (bn and bn ~= "" and bn ~= "?") then
        table.insert(lines, (wn or "?") .. "   vs   " .. (bn or "?"))
    end
    local bits = {}
    if h.Result and h.Result ~= "" and h.Result ~= "*" then
        table.insert(bits, h.Result)
    end
    if h.Date and h.Date ~= "" and h.Date ~= "?" then
        table.insert(bits, h.Date)
    end
    if h.Site and h.Site ~= "" and h.Site ~= "?" then
        table.insert(bits, h.Site)
    end
    if h.Round and h.Round ~= "" and h.Round ~= "?" then
        table.insert(bits, "Rd " .. h.Round)
    end
    if #bits > 0 then
        -- ASCII separator: Cozette + TTF_RenderText have no reliable glyph for U+00B7
        table.insert(lines, table.concat(bits, "  |  "))
    end
    if #lines == 0 then
        table.insert(lines, "PGN replay")
    end
    return lines
end

local function smoothstep(t)
    local x = math.max(0, math.min(1, t))
    return x * x * (3 - 2 * x)
end

local function set_status_line(s)
    if s == last_status_text then
        return
    end
    last_status_text = s
    modify(title_id, { content = s })
end

--------------------------------------------------------------------------------
-- Chess engine (board file 0–7 = a–h, rank 1–8; white at bottom)
--------------------------------------------------------------------------------

local function fc(f)
    return string.char(string.byte("a") + f)
end

local function sq_ok(f, r)
    return f >= 0 and f <= 7 and r >= 1 and r <= 8
end

local function copy_piece(p)
    if not p then
        return nil
    end
    return { t = p.t, w = p.w }
end

local function empty_board()
    local b = {}
    for f = 0, 7 do
        b[f] = {}
        for r = 1, 8 do
            b[f][r] = nil
        end
    end
    return b
end

local function initial_board()
    local b = empty_board()
    local back = { "R", "N", "B", "Q", "K", "B", "N", "R" }
    for f = 0, 7 do
        b[f][8] = { t = back[f + 1], w = false }
        b[f][7] = { t = "P", w = false }
        b[f][2] = { t = "P", w = true }
        b[f][1] = { t = back[f + 1], w = true }
    end
    return b
end

local function copy_game(g)
    local ng = {
        b = empty_board(),
        white = g.white,
        ep = g.ep and { f = g.ep.f, r = g.ep.r } or nil,
        ca = { g.ca[1], g.ca[2], g.ca[3], g.ca[4] },
    }
    for f = 0, 7 do
        for r = 1, 8 do
            ng.b[f][r] = copy_piece(g.b[f][r])
        end
    end
    return ng
end

local function game_new()
    return {
        b = initial_board(),
        white = true,
        ep = nil,
        ca = { true, true, true, true }, -- WKs WQs BKs BQs
    }
end

local function find_king(b, white)
    for f = 0, 7 do
        for r = 1, 8 do
            local p = b[f][r]
            if p and p.w == white and p.t == "K" then
                return f, r
            end
        end
    end
    return nil
end

local NDIRS = { { 1, 2 }, { 2, 1 }, { -1, 2 }, { -2, 1 }, { 1, -2 }, { 2, -1 }, { -1, -2 }, { -2, -1 } }
local RDIRS = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } }
local BDIRS = { { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 } }

local function ray_attack(b, pf, pr, df, dr, tf, tr)
    local f, r = pf, pr
    while true do
        f, r = f + df, r + dr
        if not sq_ok(f, r) then
            return false
        end
        if f == tf and r == tr then
            return true
        end
        if b[f][r] then
            return false
        end
    end
end

local function square_attacked_from(b, pf, pr, tf, tr)
    local p = b[pf][pr]
    if not p then
        return false
    end
    local w = p.w
    if p.t == "P" then
        if w then
            return ((tf == pf - 1 or tf == pf + 1) and tr == pr + 1)
        else
            return ((tf == pf - 1 or tf == pf + 1) and tr == pr - 1)
        end
    elseif p.t == "N" then
        for _, d in ipairs(NDIRS) do
            if pf + d[1] == tf and pr + d[2] == tr then
                return true
            end
        end
        return false
    elseif p.t == "K" then
        return math.abs(pf - tf) <= 1 and math.abs(pr - tr) <= 1 and (pf ~= tf or pr ~= tr)
    elseif p.t == "R" then
        for _, d in ipairs(RDIRS) do
            if ray_attack(b, pf, pr, d[1], d[2], tf, tr) then
                return true
            end
        end
        return false
    elseif p.t == "B" then
        for _, d in ipairs(BDIRS) do
            if ray_attack(b, pf, pr, d[1], d[2], tf, tr) then
                return true
            end
        end
        return false
    elseif p.t == "Q" then
        for _, d in ipairs(RDIRS) do
            if ray_attack(b, pf, pr, d[1], d[2], tf, tr) then
                return true
            end
        end
        for _, d in ipairs(BDIRS) do
            if ray_attack(b, pf, pr, d[1], d[2], tf, tr) then
                return true
            end
        end
        return false
    end
    return false
end

local function square_attacked(b, tf, tr, by_white)
    for f = 0, 7 do
        for r = 1, 8 do
            local p = b[f][r]
            if p and p.w == by_white and square_attacked_from(b, f, r, tf, tr) then
                return true
            end
        end
    end
    return false
end

local function in_check(g, white_king)
    local b = g.b
    local kf, kr = find_king(b, white_king)
    if not kf then
        return true
    end
    return square_attacked(b, kf, kr, not white_king)
end

local function add_move(list, from_f, from_r, to_f, to_r, promo, castle, ep)
    table.insert(list, {
        from = { f = from_f, r = from_r },
        to = { f = to_f, r = to_r },
        promo = promo,
        castle = castle,
        ep = ep,
    })
end

local function pseudo_moves(g)
    local b = g.b
    local side = g.white
    local out = {}
    for pf = 0, 7 do
        for pr = 1, 8 do
            local p = b[pf][pr]
            if p and p.w == side then
                if p.t == "P" then
                    local dir = p.w and 1 or -1
                    local start_r = p.w and 2 or 7
                    local tf, tr = pf, pr + dir
                    if sq_ok(tf, tr) and not b[tf][tr] then
                        if tr == 8 or tr == 1 then
                            for _, prm in ipairs({ "Q", "R", "B", "N" }) do
                                add_move(out, pf, pr, tf, tr, prm, nil, false)
                            end
                        else
                            add_move(out, pf, pr, tf, tr, nil, nil, false)
                            if pr == start_r then
                                local tmid, tmr = pf, pr + dir
                                local tf2, tr2 = pf, pr + 2 * dir
                                if sq_ok(tmid, tmr) and sq_ok(tf2, tr2) and not b[tmid][tmr] and not b[tf2][tr2] then
                                    add_move(out, pf, pr, tf2, tr2, nil, nil, false)
                                end
                            end
                        end
                    end
                    for df = -1, 1, 2 do
                        local cf, cr = pf + df, pr + dir
                        if sq_ok(cf, cr) then
                            local occ = b[cf][cr]
                            if occ and occ.w ~= side then
                                if cr == 8 or cr == 1 then
                                    for _, prm in ipairs({ "Q", "R", "B", "N" }) do
                                        add_move(out, pf, pr, cf, cr, prm, nil, false)
                                    end
                                else
                                    add_move(out, pf, pr, cf, cr, nil, nil, false)
                                end
                            elseif not occ and g.ep and g.ep.f == cf and g.ep.r == cr then
                                add_move(out, pf, pr, cf, cr, nil, nil, true)
                            end
                        end
                    end
                elseif p.t == "N" then
                    for _, d in ipairs(NDIRS) do
                        local tf, tr = pf + d[1], pr + d[2]
                        if sq_ok(tf, tr) then
                            local occ = b[tf][tr]
                            if not occ or occ.w ~= side then
                                add_move(out, pf, pr, tf, tr, nil, nil, false)
                            end
                        end
                    end
                elseif p.t == "K" then
                    for df = -1, 1 do
                        for dr = -1, 1 do
                            if df ~= 0 or dr ~= 0 then
                                local tf, tr = pf + df, pr + dr
                                if sq_ok(tf, tr) then
                                    local occ = b[tf][tr]
                                    if not occ or occ.w ~= side then
                                        add_move(out, pf, pr, tf, tr, nil, nil, false)
                                    end
                                end
                            end
                        end
                    end
                    -- castling
                    if not in_check(g, side) then
                        if side and pf == 4 and pr == 1 and g.ca[1] then
                            if b[5][1] == nil and b[6][1] == nil and b[7][1] and b[7][1].t == "R" and b[7][1].w then
                                local gg = copy_game(g)
                                gg.b[5][1] = gg.b[4][1]
                                gg.b[4][1] = nil
                                if not in_check(gg, true) then
                                    gg = copy_game(g)
                                    gg.b[6][1] = gg.b[4][1]
                                    gg.b[4][1] = nil
                                    if not in_check(gg, true) then
                                        add_move(out, 4, 1, 6, 1, nil, "K", false)
                                    end
                                end
                            end
                        end
                        if side and pf == 4 and pr == 1 and g.ca[2] then
                            if b[1][1] == nil and b[2][1] == nil and b[3][1] == nil and b[0][1] and b[0][1].t == "R" and b[0][1].w then
                                local gg = copy_game(g)
                                gg.b[3][1] = gg.b[4][1]
                                gg.b[4][1] = nil
                                if not in_check(gg, true) then
                                    add_move(out, 4, 1, 2, 1, nil, "Q", false)
                                end
                            end
                        end
                        if not side and pf == 4 and pr == 8 and g.ca[3] then
                            if b[5][8] == nil and b[6][8] == nil and b[7][8] and b[7][8].t == "R" and not b[7][8].w then
                                local gg = copy_game(g)
                                gg.b[5][8] = gg.b[4][8]
                                gg.b[4][8] = nil
                                if not in_check(gg, false) then
                                    gg = copy_game(g)
                                    gg.b[6][8] = gg.b[4][8]
                                    gg.b[4][8] = nil
                                    if not in_check(gg, false) then
                                        add_move(out, 4, 8, 6, 8, nil, "K", false)
                                    end
                                end
                            end
                        end
                        if not side and pf == 4 and pr == 8 and g.ca[4] then
                            if b[1][8] == nil and b[2][8] == nil and b[3][8] == nil and b[0][8] and b[0][8].t == "R" and not b[0][8].w then
                                local gg = copy_game(g)
                                gg.b[3][8] = gg.b[4][8]
                                gg.b[4][8] = nil
                                if not in_check(gg, false) then
                                    add_move(out, 4, 8, 2, 8, nil, "Q", false)
                                end
                            end
                        end
                    end
                else
                    local dirs = (p.t == "R") and RDIRS or (p.t == "B") and BDIRS or nil
                    if dirs then
                        for _, d in ipairs(dirs) do
                            local f, r = pf, pr
                            while true do
                                f, r = f + d[1], r + d[2]
                                if not sq_ok(f, r) then
                                    break
                                end
                                local occ = b[f][r]
                                if occ then
                                    if occ.w ~= side then
                                        add_move(out, pf, pr, f, r, nil, nil, false)
                                    end
                                    break
                                end
                                add_move(out, pf, pr, f, r, nil, nil, false)
                            end
                        end
                    elseif p.t == "Q" then
                        for _, d in ipairs(RDIRS) do
                            local f, r = pf, pr
                            while true do
                                f, r = f + d[1], r + d[2]
                                if not sq_ok(f, r) then
                                    break
                                end
                                local occ = b[f][r]
                                if occ then
                                    if occ.w ~= side then
                                        add_move(out, pf, pr, f, r, nil, nil, false)
                                    end
                                    break
                                end
                                add_move(out, pf, pr, f, r, nil, nil, false)
                            end
                        end
                        for _, d in ipairs(BDIRS) do
                            local f, r = pf, pr
                            while true do
                                f, r = f + d[1], r + d[2]
                                if not sq_ok(f, r) then
                                    break
                                end
                                local occ = b[f][r]
                                if occ then
                                    if occ.w ~= side then
                                        add_move(out, pf, pr, f, r, nil, nil, false)
                                    end
                                    break
                                end
                                add_move(out, pf, pr, f, r, nil, nil, false)
                            end
                        end
                    end
                end
            end
        end
    end
    return out
end

local function apply_move(g, m)
    local b = g.b
    local pf, pr, tf, tr = m.from.f, m.from.r, m.to.f, m.to.r
    local piece = b[pf][pr]
    local victim

    if m.castle == "K" then
        if piece.w then
            b[6][1] = piece
            b[4][1] = nil
            b[5][1] = b[7][1]
            b[7][1] = nil
        else
            b[6][8] = piece
            b[4][8] = nil
            b[5][8] = b[7][8]
            b[7][8] = nil
        end
    elseif m.castle == "Q" then
        if piece.w then
            b[2][1] = piece
            b[4][1] = nil
            b[3][1] = b[0][1]
            b[0][1] = nil
        else
            b[2][8] = piece
            b[4][8] = nil
            b[3][8] = b[0][8]
            b[0][8] = nil
        end
    else
        if not m.ep then
            victim = b[tf][tr]
        end
        if m.ep then
            if piece.w then
                b[tf][tr - 1] = nil
            else
                b[tf][tr + 1] = nil
            end
        end
        if m.promo then
            b[tf][tr] = { t = m.promo, w = piece.w }
        else
            b[tf][tr] = piece
        end
        b[pf][pr] = nil
        if victim and victim.t == "R" then
            if tf == 7 and tr == 1 and victim.w then
                g.ca[1] = false
            end
            if tf == 0 and tr == 1 and victim.w then
                g.ca[2] = false
            end
            if tf == 7 and tr == 8 and not victim.w then
                g.ca[3] = false
            end
            if tf == 0 and tr == 8 and not victim.w then
                g.ca[4] = false
            end
        end
    end

    g.ep = nil
    if piece.t == "P" and math.abs(tr - pr) == 2 then
        g.ep = { f = pf, r = (pr + tr) / 2 }
    end

    if piece.t == "K" then
        if piece.w then
            g.ca[1], g.ca[2] = false, false
        else
            g.ca[3], g.ca[4] = false, false
        end
    end

    if piece.t == "R" then
        if pf == 0 and pr == 1 then
            g.ca[2] = false
        end
        if pf == 7 and pr == 1 then
            g.ca[1] = false
        end
        if pf == 0 and pr == 8 then
            g.ca[4] = false
        end
        if pf == 7 and pr == 8 then
            g.ca[3] = false
        end
    end

    g.white = not g.white
end

local function legal_moves(g)
    local side = g.white
    local acc = {}
    for _, m in ipairs(pseudo_moves(g)) do
        local gg = copy_game(g)
        apply_move(gg, m)
        if not in_check(gg, side) then
            table.insert(acc, m)
        end
    end
    return acc
end

local function norm_san(s)
    s = s:gsub("^%s+", ""):gsub("%s+$", "")
    s = s:gsub("[%+#?!]+$", "")
    s = s:gsub("0%-0%-0", "O-O-O")
    s = s:gsub("0%-0", "O-O")
    return s
end

local function san_from_move(g, m)
    local leg = legal_moves(g)
    local b = g.b
    local pf, pr, tf, tr = m.from.f, m.from.r, m.to.f, m.to.r
    local piece = b[pf][pr]
    if not piece then
        return "?"
    end

    if m.castle == "K" then
        return "O-O"
    elseif m.castle == "Q" then
        return "O-O-O"
    end

    local dest = fc(tf) .. tostring(tr)
    local cap_sq = m.ep or (b[tf][tr] ~= nil)
    local letter = piece.t == "P" and "" or piece.t

    if piece.t == "P" then
        local s
        if cap_sq then
            s = fc(pf) .. "x" .. dest
        else
            s = dest
        end
        if m.promo then
            s = s .. "=" .. m.promo
        end
        return s
    end

    local others = {}
    for _, om in ipairs(leg) do
        if om.to.f == tf and om.to.r == tr and not (om.from.f == pf and om.from.r == pr) then
            local op = b[om.from.f][om.from.r]
            if op and op.t == piece.t and op.w == piece.w then
                table.insert(others, om)
            end
        end
    end

    local disc = ""
    if #others > 0 then
        local same_file, same_rank = false, false
        for _, om in ipairs(others) do
            if om.from.f == pf then
                same_file = true
            end
            if om.from.r == pr then
                same_rank = true
            end
        end
        if same_file and same_rank then
            disc = fc(pf) .. tostring(pr)
        elseif same_file then
            disc = tostring(pr)
        elseif same_rank then
            disc = fc(pf)
        else
            disc = fc(pf)
        end
    end

    local mid = cap_sq and "x" or ""
    return letter .. disc .. mid .. dest
end

local function san_match(g, token)
    local want = norm_san(token)
    for _, m in ipairs(legal_moves(g)) do
        if san_from_move(g, m) == want then
            return m
        end
    end
    return nil
end

--------------------------------------------------------------------------------
-- Rendering
--------------------------------------------------------------------------------

local function square_top_left(file, rank)
    return ORIG_X + file * CELL, BOARD_ORIG_Y + (8 - rank) * CELL
end

local function piece_top_left(file, rank, sz)
    return ORIG_X + file * CELL + (CELL - sz) / 2, BOARD_ORIG_Y + (8 - rank) * CELL + (CELL - sz) / 2
end

local function piece_center_xy(file, rank, sz)
    local px, py = piece_top_left(file, rank, sz)
    return px + sz / 2, py + sz / 2
end

local function key_at(f, r)
    return tostring(f) .. "," .. tostring(r)
end

local function piece_tex(p, has_br)
    local c = p.w and "w" or "b"
    local t = p.t
    local src = PIECE_DIR .. c .. t .. ".png"
    local flt
    if t == "R" and not p.w and not has_br then
        src = PIECE_DIR .. "wR.png"
        flt = filter.negative
    end
    return src, flt
end

local function sync_entities_from_board(g, sz, has_br)
    for f = 0, 7 do
        for r = 1, 8 do
            local k = key_at(f, r)
            local id = piece_entity[k]
            local p = g.b[f][r]
            if id then
                if p then
                    local src, flt = piece_tex(p, has_br)
                    local x, y = piece_top_left(f, r, sz)
                    local pr = { x = x, y = y, w = sz, h = sz, src = src }
                    if flt then
                        pr.filter = flt
                    end
                    modify(id, pr)
                else
                    modify(id, { w = 0, h = 0 })
                end
            end
        end
    end
end

function setup()
    squares = {}
    piece_entity = {}

    local seq = {}
    local run = load_pgn_text()
    local ok, err = pcall(function()
        local toks = tokenize_movetext(run)
        local g0 = game_new()
        for _, t in ipairs(toks) do
            local mm = san_match(g0, t)
            if not mm then
                error("illegal or unsupported SAN: " .. t)
            end
            table.insert(seq, mm)
            apply_move(g0, mm)
        end
    end)
    if not ok then
        seq = {}
        local toks = tokenize_movetext(EMBEDDED_PGN)
        local g0 = game_new()
        for _, t in ipairs(toks) do
            local mm = san_match(g0, t)
            if not mm then
                error("embedded PGN invalid at '" .. t .. "': " .. tostring(err))
            end
            table.insert(seq, mm)
            apply_move(g0, mm)
        end
    end

    _G._pgn_moves = seq

    game = game_new()
    move_index = 0
    anim = nil
    last_status_text = nil
    quit_after_deadline_ms = nil

    local hdr_src = ok and run or EMBEDDED_PGN
    local headers = parse_pgn_headers(hdr_src)
    local hlines = header_display_lines(headers)
    header_ids = {}
    for i, line in ipairs(hlines) do
        table.insert(
            header_ids,
            text({
                x = ORIG_X,
                y = HEADER_PAD_TOP + (i - 1) * HEADER_LINE_STEP,
                font = font.cozette,
                size = HEADER_FONT_SIZE,
                fg = (i == 1) and "#f2e8dc" or "#bfb4a8",
                bg = "#1e1812",
                content = line,
            })
        )
    end
    local header_bottom
    if #hlines == 0 then
        header_bottom = HEADER_PAD_TOP
    else
        header_bottom = HEADER_PAD_TOP + (#hlines - 1) * HEADER_LINE_STEP + HEADER_FONT_SIZE + 6
    end
    title_y = header_bottom + TITLE_BELOW_HEADERS
    BOARD_ORIG_Y = title_y + TITLE_BAND + HEADER_GAP_BEFORE_BOARD

    local frame = 4
    local fx, fy = ORIG_X - frame, BOARD_ORIG_Y - frame
    local fw, fh = BOARD + frame * 2, BOARD + frame * 2
    rectangle({ x = fx, y = fy, w = fw, h = fh, color = "#4a3728", motion = "none" })
    line({ x1 = fx, y1 = fy, x2 = fx + fw, y2 = fy, color = "#6b5344", thickness = 2 })
    line({ x1 = fx, y1 = fy + fh, x2 = fx + fw, y2 = fy + fh, color = "#6b5344", thickness = 2 })
    line({ x1 = fx, y1 = fy, x2 = fx, y2 = fy + fh, color = "#6b5344", thickness = 2 })
    line({ x1 = fx + fw, y1 = fy, x2 = fx + fw, y2 = fy + fh, color = "#6b5344", thickness = 2 })

    for rank = 1, 8 do
        for file = 0, 7 do
            local is_dark = (file + rank) % 2 == 1
            local x, y = square_top_left(file, rank)
            table.insert(squares, {
                id = square({ x = x, y = y, size = CELL, color = hex(is_dark and BASE_DARK or BASE_LIGHT), motion = "none" }),
                file = file,
                rank = rank,
                dark = is_dark,
            })
        end
    end

    local label_color = "#c4b5a0"
    local label_size = 18
    for file = 0, 7 do
        text({
            x = ORIG_X + file * CELL + CELL / 2 - 5,
            y = BOARD_ORIG_Y + BOARD + 6,
            font = font.cozette,
            size = label_size,
            fg = label_color,
            bg = "#1e1812",
            content = string.char(string.byte("a") + file),
        })
    end
    for rank = 1, 8 do
        text({
            x = ORIG_X - 16,
            y = BOARD_ORIG_Y + (8 - rank) * CELL + CELL / 2 - 9,
            font = font.cozette,
            size = label_size,
            fg = label_color,
            bg = "#1e1812",
            content = tostring(rank),
        })
    end

    title_id = text({
        x = ORIG_X + 8,
        y = title_y,
        font = font.cozette,
        size = 17,
        fg = "#a89888",
        bg = "#1e1812",
        content = "--",
    })

    local has_br = file_exists(PIECE_DIR .. "bR.png")
    local sz = math.floor(CELL * PIECE_SCALE)

    piece_entity = {}
    for f = 0, 7 do
        for r = 1, 8 do
            local p = game.b[f][r]
            if p then
                local src, flt = piece_tex(p, has_br)
                local x, y = piece_top_left(f, r, sz)
                local opts = { x = x, y = y, w = sz, h = sz, src = src, motion = "none" }
                if flt then
                    opts.filter = flt
                end
                local id = image(opts)
                piece_entity[key_at(f, r)] = id
            end
        end
    end

    _G._pgn_has_br = has_br
    _G._piece_sz = sz
end

function update(dt)
    local seq = _G._pgn_moves
    local sz = _G._piece_sz
    local has_br = _G._pgn_has_br
    if not seq then
        return
    end

    local t = time() * 0.001

    for i = 1, #squares do
        local s = squares[i]
        local wobble = math.sin(t * 1.4 + s.file * 0.4 + s.rank * 0.4) * 10
        local base = s.dark and BASE_DARK or BASE_LIGHT
        modify(s.id, {
            color = hex({
                r = math.max(0, math.min(255, base.r + wobble)),
                g = math.max(0, math.min(255, base.g + wobble * 0.75)),
                b = math.max(0, math.min(255, base.b + wobble * 0.45)),
            }),
        })
    end

    if anim then
        anim.elapsed = anim.elapsed + dt
        local u_raw = math.min(1, anim.elapsed / SECONDS_PER_HALFMOVE)
        local u = smoothstep(u_raw)
        for _, o in ipairs(anim.objs) do
            local x = o.x0 + (o.x1 - o.x0) * u
            local y = o.y0 + (o.y1 - o.y0) * u
            modify(o.id, { x = x, y = y, w = sz, h = sz })
        end
        -- Captured / en-passant piece: hold through contact, then shrink with a small drop
        local t_sh0, t_sh1 = 0.34, 0.68
        for _, cap in ipairs(anim.captures or {}) do
            local sc = 1
            if u_raw >= t_sh1 then
                sc = 0
            elseif u_raw > t_sh0 then
                sc = 1 - smoothstep((u_raw - t_sh0) / (t_sh1 - t_sh0))
            end
            local w = math.max(0, math.floor(cap.sz * sc + 0.5))
            if w < 2 then
                modify(cap.id, { w = 0, h = 0 })
            else
                local drop = (1 - sc) * 14
                modify(cap.id, {
                    x = cap.cx - w / 2,
                    y = cap.cy - w / 2 + drop,
                    w = w,
                    h = w,
                })
            end
        end
        if u_raw >= 1 then
            game = anim.after
            move_index = anim.new_index
            sync_entities_from_board(game, sz, has_br)
            anim = nil
        end
        if #seq > 0 then
            set_status_line(string.format("Moves  %d / %d", move_index + 1, #seq))
        else
            set_status_line("No moves")
        end
        return
    end

    -- start next half-move
    if move_index < #seq then
        local m = seq[move_index + 1]
        local g_before = copy_game(game)
        local pf, pr, tf, tr = m.from.f, m.from.r, m.to.f, m.to.r
        local kf, kt = key_at(pf, pr), key_at(tf, tr)
        local id_move = piece_entity[kf]
        if not id_move then
            move_index = move_index + 1
            apply_move(game, m)
            return
        end

        local x0, y0 = piece_top_left(pf, pr, sz)
        local x1, y1 = piece_top_left(tf, tr, sz)
        local objs = { { id = id_move, x0 = x0, y0 = y0, x1 = x1, y1 = y1 } }

        if m.castle == "K" then
            local rf, rr = 7, pr
            if not g_before.b[pf][pr].w then
                rr = 8
            end
            local rk = key_at(7, rr)
            local rid = piece_entity[rk]
            if rid then
                local rx0, ry0 = piece_top_left(7, rr, sz)
                local rx1, ry1 = piece_top_left(5, rr, sz)
                table.insert(objs, { id = rid, x0 = rx0, y0 = ry0, x1 = rx1, y1 = ry1 })
            end
        elseif m.castle == "Q" then
            local rr = pr
            local rk = key_at(0, rr)
            local rid = piece_entity[rk]
            if rid then
                local rx0, ry0 = piece_top_left(0, rr, sz)
                local rx1, ry1 = piece_top_left(3, rr, sz)
                table.insert(objs, { id = rid, x0 = rx0, y0 = ry0, x1 = rx1, y1 = ry1 })
            end
        end

        local g_after = copy_game(g_before)
        apply_move(g_after, m)

        local capt_list = {}
        if m.ep then
            local vf, vr
            if g_before.b[pf][pr].w then
                vf, vr = tf, tr - 1
            else
                vf, vr = tf, tr + 1
            end
            local vk = key_at(vf, vr)
            local vid = piece_entity[vk]
            if vid then
                local cx, cy = piece_center_xy(vf, vr, sz)
                table.insert(capt_list, { id = vid, cx = cx, cy = cy, sz = sz })
            end
        else
            local cap = g_before.b[tf][tr]
            if cap and not m.castle then
                local cid = piece_entity[kt]
                if cid then
                    local cx, cy = piece_center_xy(tf, tr, sz)
                    table.insert(capt_list, { id = cid, cx = cx, cy = cy, sz = sz })
                end
            end
        end

        piece_entity[kt] = id_move
        piece_entity[kf] = nil
        if m.castle == "K" then
            local rr = g_before.b[pf][pr].w and 1 or 8
            local rpk, rtk = key_at(7, rr), key_at(5, rr)
            piece_entity[rtk] = piece_entity[rpk]
            piece_entity[rpk] = nil
        elseif m.castle == "Q" then
            local rr = g_before.b[pf][pr].w and 1 or 8
            local rpk, rtk = key_at(0, rr), key_at(3, rr)
            piece_entity[rtk] = piece_entity[rpk]
            piece_entity[rpk] = nil
        end

        if m.promo then
            local src, flt = piece_tex({ t = m.promo, w = g_before.b[pf][pr].w }, has_br)
            modify(id_move, { src = src, filter = flt })
        end

        anim = {
            elapsed = 0,
            objs = objs,
            captures = capt_list,
            after = g_after,
            new_index = move_index + 1,
        }
    else
        set_status_line(string.format("Complete  (%d half-moves)", #seq))
    end

    if not anim then
        if #seq == 0 then
            set_status_line("No moves")
        else
            set_status_line(string.format("Moves  %d / %d", math.min(move_index + 1, #seq), #seq))
        end
    end

    if QUIT_AFTER_DONE_S and QUIT_AFTER_DONE_S > 0 and type(quit) == "function" then
        if not anim and #seq > 0 and move_index >= #seq then
            if not quit_after_deadline_ms then
                quit_after_deadline_ms = time() + QUIT_AFTER_DONE_S * 1000
            end
            if time() >= quit_after_deadline_ms then
                quit()
            end
        end
    end
end

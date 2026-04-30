-- Chess board: standard start position, white knight on g1 touring a closed path, soft square pulse.
window(800, 800)
bg("#1e1812")
fps(30)
seed(7)

local CELL = 72
local PIECE_SCALE = 0.88
local BOARD = 8 * CELL
local ORIG_X = (800 - BOARD) / 2
local ORIG_Y = (800 - BOARD) / 2 + 20

-- Classic light/dark board (similar to common wood themes)
local BASE_LIGHT = { r = 240, g = 217, b = 181 }
local BASE_DARK = { r = 181, g = 133, b = 99 }

-- Closed knight path: g1 → e2 → c1 → a2 → b4 → d3 → f4 → h3 → g1 (files 0–7, ranks 1–8)
local KNIGHT_PATH = {
    { 6, 1 },
    { 4, 2 },
    { 2, 1 },
    { 0, 2 },
    { 1, 4 },
    { 3, 3 },
    { 5, 4 },
    { 7, 3 },
}

local squares = {}
local knight_id

function square_top_left(file, rank)
    return ORIG_X + file * CELL, ORIG_Y + (8 - rank) * CELL
end

function piece_top_left(file, rank, sz)
    return ORIG_X + file * CELL + (CELL - sz) / 2, ORIG_Y + (8 - rank) * CELL + (CELL - sz) / 2
end

function setup()
    -- Frame
    local frame = 4
    local fx = ORIG_X - frame
    local fy = ORIG_Y - frame
    local fw = BOARD + frame * 2
    local fh = BOARD + frame * 2
    rectangle({ x = fx, y = fy, w = fw, h = fh, color = "#4a3728", motion = "none" })
    line({ x1 = fx, y1 = fy, x2 = fx + fw, y2 = fy, color = "#6b5344", thickness = 2 })
    line({ x1 = fx, y1 = fy + fh, x2 = fx + fw, y2 = fy + fh, color = "#6b5344", thickness = 2 })
    line({ x1 = fx, y1 = fy, x2 = fx, y2 = fy + fh, color = "#6b5344", thickness = 2 })
    line({ x1 = fx + fw, y1 = fy, x2 = fx + fw, y2 = fy + fh, color = "#6b5344", thickness = 2 })

    -- Squares
    for rank = 1, 8 do
        for file = 0, 7 do
            local is_dark = (file + rank) % 2 == 1
            local x, y = square_top_left(file, rank)
            local id = square({
                x = x,
                y = y,
                size = CELL,
                color = hex(is_dark and BASE_DARK or BASE_LIGHT),
                motion = "none",
            })
            table.insert(squares, {
                id = id,
                file = file,
                rank = rank,
                dark = is_dark,
            })
        end
    end

    -- Rank / file labels
    local label_color = "#c4b5a0"
    local label_size = 18
    for file = 0, 7 do
        local x = ORIG_X + file * CELL + CELL / 2 - 5
        local y = ORIG_Y + BOARD + 6
        text({
            x = x,
            y = y,
            font = font.cozette,
            size = label_size,
            fg = label_color,
            bg = "#1e1812",
            content = string.char(string.byte("a") + file),
        })
    end
    for rank = 1, 8 do
        local x = ORIG_X - 16
        local y = ORIG_Y + (8 - rank) * CELL + CELL / 2 - 9
        text({
            x = x,
            y = y,
            font = font.cozette,
            size = label_size,
            fg = label_color,
            bg = "#1e1812",
            content = tostring(rank),
        })
    end

    -- Title
    text({
        x = ORIG_X + 8,
        y = 12,
        font = font.cozette,
        size = 22,
        fg = "#e8dcc8",
        bg = "#1e1812",
        content = "Knight circuit",
        blend = true,
    })

    -- PNG sprites under assets/pieces. If bR.png is missing, black rooks use wR + negative.
    local PIECE_DIR = "./assets/pieces/"
    local has_black_rook_png = file_exists(PIECE_DIR .. "bR.png")
    local sz = math.floor(CELL * PIECE_SCALE)

    local function place_piece(file, rank, code, is_white)
        local x, y = piece_top_left(file, rank, sz)
        local prefix = is_white and "w" or "b"
        local src = PIECE_DIR .. prefix .. code .. ".png"
        local opts = {
            x = x,
            y = y,
            w = sz,
            h = sz,
            src = src,
            motion = "none",
        }
        if code == "R" and not is_white and not has_black_rook_png then
            opts.src = PIECE_DIR .. "wR.png"
            opts.filter = filter.negative
        end
        image(opts)
    end

    local back_row = { "R", "N", "B", "Q", "K", "B", "N", "R" }
    for file = 0, 7 do
        place_piece(file, 8, back_row[file + 1], false)
        place_piece(file, 7, "P", false)
        place_piece(file, 2, "P", true)
        -- g1 (file 6) knight is animated along KNIGHT_PATH; skip duplicate static sprite
        if not (file == 6 and back_row[file + 1] == "N") then
            place_piece(file, 1, back_row[file + 1], true)
        end
    end

    local kx, ky = piece_top_left(6, 1, sz)
    knight_id = image({
        x = kx,
        y = ky,
        w = sz,
        h = sz,
        src = PIECE_DIR .. "wN.png",
        motion = "none",
    })
end

function update(dt)
    local t = time() * 0.001

    -- Breathing colors on each square
    for i = 1, #squares do
        local s = squares[i]
        local amp = s.dark and 10 or 14
        local wobble = math.sin(t * 1.8 + s.file * 0.45 + s.rank * 0.45) * amp
        local base = s.dark and BASE_DARK or BASE_LIGHT
        modify(s.id, {
            color = hex({
                r = math.max(0, math.min(255, base.r + wobble)),
                g = math.max(0, math.min(255, base.g + wobble * 0.8)),
                b = math.max(0, math.min(255, base.b + wobble * 0.5)),
            }),
        })
    end

    -- g1 knight: smooth travel along each edge of KNIGHT_PATH (closed loop)
    local sz = math.floor(CELL * PIECE_SCALE)
    local n = #KNIGHT_PATH
    local seg_dur = 0.85
    local cycle = seg_dur * n
    local cycle_t = t % cycle
    local seg = math.min(math.floor(cycle_t / seg_dur) + 1, n)
    local frac = (cycle_t % seg_dur) / seg_dur

    local k1 = seg
    local k2 = seg % n + 1

    local f1, r1 = KNIGHT_PATH[k1][1], KNIGHT_PATH[k1][2]
    local f2, r2 = KNIGHT_PATH[k2][1], KNIGHT_PATH[k2][2]

    local x1, y1 = piece_top_left(f1, r1, sz)
    local x2, y2 = piece_top_left(f2, r2, sz)
    local hx = x1 + (x2 - x1) * frac
    local hy = y1 + (y2 - y1) * frac

    modify(knight_id, { x = hx, y = hy })
end

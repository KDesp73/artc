-- Knight tour + kings + bishop; paths stay on disjoint squares.
local W, H = 800, 800
window(W, H)
bg("#121218")
fps(45)
seed(7)

local CELL = math.floor(math.min(W, H) / 8)
local PIECE_SCALE = 0.88
local BOARD = CELL * 8
local ORIG_X = (W - BOARD) / 2
local ORIG_Y = (H - BOARD) / 2

local BASE_LIGHT = { r = 232, g = 228, b = 218 }
local BASE_DARK = { r = 58, g = 82, b = 74 }

-- Knight closed tour (vertices — no other pieces use these squares)
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

-- White king: small orbit on e1–d1–f1 (never visits knight squares)
local WK_PATH = {
    { 4, 1 },
    { 3, 1 },
    { 4, 1 },
    { 5, 1 },
    { 4, 1 },
}

-- Black king: trace e8–h8 and back (knight never enters rank 8 on this tour)
local BK_PATH = {
    { 4, 8 },
    { 5, 8 },
    { 6, 8 },
    { 7, 8 },
    { 6, 8 },
    { 5, 8 },
    { 4, 8 },
}

-- Black bishop: one diagonal step per edge; large loop mid-board (no knight / king squares)
local BB_PATH = {
    { 2, 5 },
    { 3, 6 },
    { 4, 7 },
    { 5, 6 },
    { 6, 5 },
    { 7, 4 },
    { 6, 3 },
    { 5, 2 },
    { 4, 3 },
    { 3, 4 },
}

local squares = {}
--- movers: id, path, seg_dur, arc (knight jump), phase (time offset)
local movers = {}

local function smootherstep(t)
    local x = math.max(0, math.min(1, t))
    return x * x * x * (x * (x * 6 - 15) + 10)
end

function square_top_left(file, rank)
    return ORIG_X + file * CELL, ORIG_Y + (8 - rank) * CELL
end

function piece_top_left(file, rank, sz)
    return ORIG_X + file * CELL + (CELL - sz) / 2, ORIG_Y + (8 - rank) * CELL + (CELL - sz) / 2
end

function setup()
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

    local PIECE_DIR = "./assets/pieces/"
    local has_bb_png = file_exists(PIECE_DIR .. "bB.png")
    local sz = math.floor(CELL * PIECE_SCALE)

    local function add_piece(src, filter, path, seg_dur, arc, phase)
        local f0, r0 = path[1][1], path[1][2]
        local x0, y0 = piece_top_left(f0, r0, sz)
        local opts = { x = x0, y = y0, w = sz, h = sz, src = src, motion = "none" }
        if filter then
            opts.filter = filter
        end
        local id = image(opts)
        table.insert(movers, {
            id = id,
            path = path,
            seg_dur = seg_dur,
            arc = arc,
            phase = phase or 0,
        })
        return id
    end

    add_piece(PIECE_DIR .. "wN.png", nil, KNIGHT_PATH, 0.72, true, 0)
    add_piece(PIECE_DIR .. "wK.png", nil, WK_PATH, 1.05, false, 0.2)
    add_piece(PIECE_DIR .. "bK.png", nil, BK_PATH, 1.35, false, 0.55)
    local bb_src = PIECE_DIR .. "bB.png"
    local bb_filt = nil
    if not has_bb_png then
        bb_src = PIECE_DIR .. "wB.png"
        bb_filt = filter.negative
    end
    -- Slightly quicker segments so the long loop completes in a pleasant time
    add_piece(bb_src, bb_filt, BB_PATH, 0.52, false, 0.78)
end

local function move_piece(m, sz, t)
    local path = m.path
    local n = #path
    local seg_dur = m.seg_dur
    local cycle = seg_dur * n
    local cycle_t = (t + m.phase) % cycle
    local seg = math.min(math.floor(cycle_t / seg_dur) + 1, n)
    local raw_u = (cycle_t % seg_dur) / seg_dur
    local u = smootherstep(raw_u)

    local k1 = seg
    local k2 = seg % n + 1

    local f1, r1 = path[k1][1], path[k1][2]
    local f2, r2 = path[k2][1], path[k2][2]

    local x1, y1 = piece_top_left(f1, r1, sz)
    local x2, y2 = piece_top_left(f2, r2, sz)
    local dx, dy = x2 - x1, y2 - y1

    local mx = x1 + dx * u
    local my = y1 + dy * u

    if m.arc then
        local span = math.max(36, math.sqrt(dx * dx + dy * dy))
        local arc_h = math.sin(u * math.pi) * math.min(38, span * 0.22)
        my = my - arc_h
        local squash = 1 + math.sin(u * math.pi) * 0.06
        local dw = math.floor(sz * squash + 0.5)
        local dh = math.floor(sz / squash + 0.5)
        local ox = (sz - dw) * 0.5
        local oy = (sz - dh) * 0.5
        modify(m.id, { x = mx + ox, y = my + oy, w = dw, h = dh })
    else
        -- Kings / bishop: slight vertical bob while sliding (readable “step”)
        local bob = math.sin(u * math.pi) * 5
        my = my - bob
        modify(m.id, { x = mx, y = my, w = sz, h = sz })
    end
end

function update(dt)
    local t = time() * 0.001

    for i = 1, #squares do
        local s = squares[i]
        local amp = s.dark and 6 or 9
        local wobble = math.sin(t * 1.1 + s.file * 0.31 + s.rank * 0.31) * amp
        local base = s.dark and BASE_DARK or BASE_LIGHT
        modify(s.id, {
            color = hex({
                r = math.max(0, math.min(255, base.r + wobble)),
                g = math.max(0, math.min(255, base.g + wobble * 0.75)),
                b = math.max(0, math.min(255, base.b + wobble * 0.5)),
            }),
        })
    end

    local sz = math.floor(CELL * PIECE_SCALE)
    for i = 1, #movers do
        move_piece(movers[i], sz, t)
    end
end

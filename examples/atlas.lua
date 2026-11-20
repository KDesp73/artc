-- atlas.lua — 4D attractor observatory with a live flow-field mesh.
--
-- Usage:
--   artc examples/atlas.lua
--   artc examples/atlas.lua -- mode=rossler seed=99
--   artc examples/atlas.lua -- ./data/points.csv seed=7
--   artc examples/atlas.lua -- dataset=./data/points.csv
--
-- Script args are parsed as key=value pairs; any bare token is treated as a
-- CSV path. CSV rows are `x,y,z[,w]`; they are normalised before projection.

window(1200, 800)
bg("#05060a")
fps(30)
palette("catppuccin")
seed(7)

local W, H = 1200, 800
local HIDDEN = "#05060a"
local TAU = math.pi * 2

--------------------------------------------------------------------------------
-- CLI options
--------------------------------------------------------------------------------

local OPT = { mode = "lorenz", seed = 7, dataset = nil }

local function parse_args()
    local n = (type(script_argc) == "function") and script_argc() or 0
    for i = 1, n do
        local token = arg[i] or ""
        local key, value = token:match("^([%w_]+)=(.*)$")
        if key == "mode" then
            OPT.mode = value
        elseif key == "seed" then
            OPT.seed = tonumber(value) or OPT.seed
        elseif key == "dataset" then
            OPT.dataset = value
        elseif token ~= "" then
            OPT.dataset = token
        end
    end
end

parse_args()
seed(OPT.seed)

--------------------------------------------------------------------------------
-- Layout / entity budget
--------------------------------------------------------------------------------

local MAIN = { x = 24, y = 96, w = 828, h = 680 }
local SIDE = { x = 876, y = 96, w = 300, h = 680 }
local MAIN_CX = MAIN.x + MAIN.w / 2
local MAIN_CY = MAIN.y + MAIN.h / 2

local ATTRACTOR_SEGMENTS = 720
local PARTICLE_COUNT = 320
local MESH_SEGMENTS = 900
local FRACTAL_SEGMENTS = 511 -- 2^9 - 1
local MESH_RADIUS = 58
local CELL = 48
local GRID_COLS = math.ceil(MAIN.w / CELL)
local GRID_ROWS = math.ceil(MAIN.h / CELL)

local attractor_points = {}
local attractor_lines = {}
local particles = {}
local mesh_lines = {}
local fractal_lines = {}
local hud = {}
local frame = {}

local mesh_used_prev = 0
local dataset_label = "procedural"
local image_id = nil

local FILTERS = { filter.greyscale, filter.sepia, filter.negative, filter.threshold }
local FILTER_LABELS = { "GREYSCALE", "SEPIA", "NEGATIVE", "THRESHOLD" }
local filter_index = 1

local IMAGE_SRC = "./examples/grandma.jpg"

--------------------------------------------------------------------------------
-- Helpers
--------------------------------------------------------------------------------

local function clamp(v, lo, hi)
    if v < lo then return lo end
    if v > hi then return hi end
    return v
end

local function hsla(h, s, l, a)
    local rgb = color.to_rgb(color.hsl(h % 360, s, l))
    return color.rgba(rgb.r, rgb.g, rgb.b, clamp(math.floor(a), 0, 255))
end

local function push_point(list, x, y, z, w)
    list[#list + 1] = { x, y, z, w or 0 }
end

--------------------------------------------------------------------------------
-- Attractor sources
--------------------------------------------------------------------------------

local function build_lorenz()
    local pts = {}
    local x, y, z, w = 0.1, 0.0, 0.0, 0.0
    local dt = 0.004

    local function step()
        local dx = 10 * (y - x)
        local dy = x * (28 - z) - y
        local dz = x * y - (8 / 3) * z
        local dw = -w + math.sin(x * 0.1)

        x = x + dx * dt
        y = y + dy * dt
        z = z + dz * dt
        w = w + dw * dt
    end

    for _ = 1, 400 do step() end
    for i = 1, ATTRACTOR_SEGMENTS + 1 do
        push_point(pts, x, y, z, w)
        step()
    end

    return pts
end

local function build_rossler()
    local pts = {}
    local x, y, z, w = 0.1, 0.0, 0.0, 0.0
    local dt = 0.012

    local function step()
        local dx = -y - z
        local dy = x + 0.2 * y
        local dz = 0.2 + z * (x - 5.7)
        local dw = -w + 0.5 * math.sin(x * 0.3)

        x = x + dx * dt
        y = y + dy * dt
        z = z + dz * dt
        w = w + dw * dt
    end

    for _ = 1, 300 do step() end
    for i = 1, ATTRACTOR_SEGMENTS + 1 do
        push_point(pts, x, y, z, w)
        step()
    end

    return pts
end

local function parse_csv_line(line)
    local vals, n = {}, 0
    for token in line:gmatch("[^,;%s]+") do
        local num = tonumber(token)
        if num then
            n = n + 1
            vals[n] = num
            if n == 4 then break end
        end
    end
    if n >= 3 then
        return vals[1], vals[2], vals[3], vals[4] or 0
    end
    return nil
end

local function build_dataset(path)
    if not (type(read_file) == "function" and type(file_exists) == "function") then
        return nil
    end
    if not file_exists(path) then return nil end

    local text = read_file(path)
    if type(text) ~= "string" or text == "" then return nil end

    local raw = {}
    for line in text:gmatch("[^\r\n]+") do
        local x, y, z, w = parse_csv_line(line)
        if x then raw[#raw + 1] = { x, y, z, w } end
    end
    if #raw < 2 then return nil end

    local mins = { math.huge, math.huge, math.huge, math.huge }
    local maxs = { -math.huge, -math.huge, -math.huge, -math.huge }
    for _, p in ipairs(raw) do
        for d = 1, 4 do
            if p[d] < mins[d] then mins[d] = p[d] end
            if p[d] > maxs[d] then maxs[d] = p[d] end
        end
    end

    local pts = {}
    local span = #raw - 1
    for i = 1, ATTRACTOR_SEGMENTS + 1 do
        local src = raw[((i - 1) % span) + 1]
        local q = {}
        for d = 1, 4 do
            local range = maxs[d] - mins[d]
            local v = 0
            if range > 1e-9 then v = ((src[d] - mins[d]) / range) * 2 - 1 end
            q[d] = v * 18
        end
        pts[i] = q
    end

    return pts
end

local function build_points()
    if OPT.dataset then
        local pts = build_dataset(OPT.dataset)
        if pts then
            dataset_label = "csv:" .. tostring(OPT.dataset:match("([^/]+)$") or OPT.dataset)
            return pts
        end
        dataset_label = "csv:missing"
    end

    if OPT.mode == "rossler" then
        dataset_label = "rossler"
        return build_rossler()
    end

    dataset_label = "lorenz"
    return build_lorenz()
end

--------------------------------------------------------------------------------
-- 4D -> 2D projection
--------------------------------------------------------------------------------

local function project(p, a, b, c)
    local x, y, z, w = p[1], p[2], p[3], p[4]

    local x1 = x * math.cos(a) - w * math.sin(a)
    local w1 = x * math.sin(a) + w * math.cos(a)

    local y1 = y * math.cos(b) - z * math.sin(b)
    local z1 = y * math.sin(b) + z * math.cos(b)

    local z2 = z1 * math.cos(c) - w1 * math.sin(c)
    local w2 = z1 * math.sin(c) + w1 * math.cos(c)

    local denom = 3.6 - z2 * 0.014 - w2 * 0.010
    if denom < 0.4 then denom = 0.4 end
    local depth = 3.6 / denom

    return MAIN_CX + x1 * 7.0 * depth,
           MAIN_CY - y1 * 7.0 * depth,
           depth
end

--------------------------------------------------------------------------------
-- Setup
--------------------------------------------------------------------------------

function setup()
    local pts = build_points()
    for i = 1, #pts do attractor_points[i] = pts[i] end

    -- Panels.
    rectangle({ x = MAIN.x, y = MAIN.y, w = MAIN.w, h = MAIN.h, color = "#080b14", motion = "none" })
    rectangle({ x = SIDE.x, y = SIDE.y, w = SIDE.w, h = SIDE.h, color = "#080b14", motion = "none" })

    -- Rules.
    frame.top = line({ x1 = 24, y1 = 78, x2 = 1176, y2 = 78, color = "#243049", thickness = 1 })
    frame.divider = line({
        x1 = MAIN.x + MAIN.w + 16, y1 = MAIN.y,
        x2 = MAIN.x + MAIN.w + 16, y2 = MAIN.y + MAIN.h,
        color = "#243049", thickness = 1,
    })
    frame.side_a = line({
        x1 = SIDE.x + 8, y1 = SIDE.y + 236,
        x2 = SIDE.x + SIDE.w - 8, y2 = SIDE.y + 236,
        color = "#243049", thickness = 1,
    })
    frame.side_b = line({
        x1 = SIDE.x + 8, y1 = SIDE.y + 448,
        x2 = SIDE.x + SIDE.w - 8, y2 = SIDE.y + 448,
        color = "#243049", thickness = 1,
    })

    -- Filtered reference image (small, so the per-frame reload stays cheap).
    if type(file_exists) == "function" and file_exists(IMAGE_SRC) then
        image_id = image({
            x = SIDE.x + 18,
            y = SIDE.y + 248,
            w = 264,
            h = 158,
            src = IMAGE_SRC,
            filter = FILTERS[filter_index],
            motion = "none",
        })
    end

    -- Pool: fractal inset.
    for i = 1, FRACTAL_SEGMENTS do
        fractal_lines[i] = line({ color = HIDDEN, thickness = 1 })
    end

    -- Pool: attractor.
    for i = 1, ATTRACTOR_SEGMENTS do
        attractor_lines[i] = line({ color = HIDDEN, thickness = 1 })
    end

    -- Pool: flow-field mesh.
    for i = 1, MESH_SEGMENTS do
        mesh_lines[i] = line({ color = HIDDEN, thickness = 1 })
    end

    -- Flow particles (created last so they render above the mesh).
    for i = 1, PARTICLE_COUNT do
        local p = {
            x = rand(MAIN.x + 8, MAIN.x + MAIN.w - 8),
            y = rand(MAIN.y + 8, MAIN.y + MAIN.h - 8),
            phase = rand(0, TAU),
            size = rand(2.0, 3.6),
            gx = 1,
            gy = 1,
        }
        p.id = circle({
            x = p.x, y = p.y,
            size = p.size,
            color = "#91d7e3",
            motion = "none",
        })
        particles[i] = p
    end

    -- Text HUD.
    hud.title = text({
        x = 24, y = 16,
        font = font.cozette, size = 24,
        fg = palette.light_blue, bg = "#05060a", blend = true,
        content = "ATLAS / 4D ATTRACTOR OBSERVATORY",
    })
    hud.sub = text({
        x = 24, y = 50,
        font = font.cozette, size = 14,
        fg = palette.light_grey, bg = "#05060a", blend = true,
        content = "",
    })
    hud.mode = text({
        x = SIDE.x + 16, y = SIDE.y + 16,
        font = font.cozette, size = 15,
        fg = palette.white, bg = "#080b14", blend = true,
        content = "",
    })
    hud.growth = text({
        x = SIDE.x + 16, y = SIDE.y + 38,
        font = font.cozette, size = 13,
        fg = palette.light_grey, bg = "#080b14", blend = true,
        content = "",
    })
    hud.filter = text({
        x = SIDE.x + 16, y = SIDE.y + 424,
        font = font.cozette, size = 13,
        fg = palette.yellow, bg = "#080b14", blend = true,
        content = "FILTER: " .. FILTER_LABELS[filter_index],
    })
    hud.legend = {}
    local legend_lines = {
        "SIGNAL  4D strange attractor",
        "MESH    flow-field neighbours",
        "NODES   advected particles",
        "GROWTH  recursive L-system",
    }
    for i, label in ipairs(legend_lines) do
        hud.legend[i] = text({
            x = SIDE.x + 16, y = SIDE.y + 462 + (i - 1) * 20,
            font = font.cozette, size = 13,
            fg = palette.light_grey, bg = "#080b14", blend = true,
            content = label,
        })
    end
    hud.input = text({
        x = SIDE.x + 16, y = SIDE.y + SIDE.h - 48,
        font = font.cozette, size = 12,
        fg = palette.grey, bg = "#080b14", blend = true,
        content = "args: mode=<lorenz|rossler> seed=<n> <file.csv>",
    })
    hud.stats = text({
        x = MAIN.x + 12, y = MAIN.y + MAIN.h - 26,
        font = font.cozette, size = 14,
        fg = palette.light_grey, bg = "#080b14", blend = true,
        content = "",
    })
end

--------------------------------------------------------------------------------
-- Update layers
--------------------------------------------------------------------------------

local function update_attractor(t)
    local a = t * 0.31
    local b = t * 0.19 + 0.6
    local c = t * 0.11

    for i = 1, ATTRACTOR_SEGMENTS do
        local x1, y1, d1 = project(attractor_points[i], a, b, c)
        local x2, y2, d2 = project(attractor_points[i + 1], a, b, c)

        x1 = clamp(x1, MAIN.x + 2, MAIN.x + MAIN.w - 2)
        y1 = clamp(y1, MAIN.y + 2, MAIN.y + MAIN.h - 2)
        x2 = clamp(x2, MAIN.x + 2, MAIN.x + MAIN.w - 2)
        y2 = clamp(y2, MAIN.y + 2, MAIN.y + MAIN.h - 2)

        local depth = (d1 + d2) * 0.5
        local hue = (i / ATTRACTOR_SEGMENTS * 280 + t * 22) % 360
        local light = clamp(34 + (depth - 0.7) * 30, 28, 72)
        local alpha = clamp(48 + (depth - 0.7) * 190, 32, 240)

        modify(attractor_lines[i], {
            x1 = x1, y1 = y1,
            x2 = x2, y2 = y2,
            color = hsla(hue, 88, light, alpha),
            thickness = clamp(depth * 1.5, 1, 3.2),
        })
    end
end

local function grid_coords(p)
    return clamp(math.floor((p.x - MAIN.x) / CELL) + 1, 1, GRID_COLS),
           clamp(math.floor((p.y - MAIN.y) / CELL) + 1, 1, GRID_ROWS)
end

local function update_flow(t, dt)
    local buckets = {}

    for i = 1, #particles do
        local p = particles[i]

        local fx = math.sin((p.y - MAIN_CY) * 0.018 + t * 0.7 + p.phase)
                 + 0.35 * math.cos((p.x - MAIN_CX) * 0.012 - t * 0.4)
        local fy = math.cos((p.x - MAIN_CX) * 0.016 - t * 0.6 + p.phase)
                 - 0.35 * math.sin((p.y - MAIN_CY) * 0.011 + t * 0.3)

        p.vx, p.vy = fx, fy
        p.x = p.x + fx * 34 * dt
        p.y = p.y + fy * 34 * dt

        local margin = 4
        if p.x < MAIN.x + margin then
            p.x = MAIN.x + MAIN.w - margin
        elseif p.x > MAIN.x + MAIN.w - margin then
            p.x = MAIN.x + margin
        end
        if p.y < MAIN.y + margin then
            p.y = MAIN.y + MAIN.h - margin
        elseif p.y > MAIN.y + MAIN.h - margin then
            p.y = MAIN.y + margin
        end

        p.gx, p.gy = grid_coords(p)
        local key = p.gx * 100 + p.gy
        local bucket = buckets[key]
        if not bucket then
            bucket = {}
            buckets[key] = bucket
        end
        bucket[#bucket + 1] = i

        local mag = math.sqrt(fx * fx + fy * fy)
        local hue = (182 + mag * 46 + t * 12) % 360
        local light = 52 + math.sin(t * 2 + p.phase) * 8

        modify(p.id, {
            x = p.x,
            y = p.y,
            size = p.size + mag * 0.6,
            color = hsla(hue, 85, light, 220),
        })
    end

    local used = 0
    local radius_sq = MESH_RADIUS * MESH_RADIUS

    for i = 1, #particles do
        local p = particles[i]
        local b1j, b1d = 0, math.huge
        local b2j, b2d = 0, math.huge

        for ox = -1, 1 do
            for oy = -1, 1 do
                local list = buckets[(p.gx + ox) * 100 + (p.gy + oy)]
                if list then
                    for k = 1, #list do
                        local j = list[k]
                        if j > i then
                            local q = particles[j]
                            local dx, dy = q.x - p.x, q.y - p.y
                            local d2 = dx * dx + dy * dy
                            if d2 < radius_sq then
                                if d2 < b1d then
                                    b2j, b2d = b1j, b1d
                                    b1j, b1d = j, d2
                                elseif d2 < b2d then
                                    b2j, b2d = j, d2
                                end
                            end
                        end
                    end
                end
            end
        end

        if b1j > 0 and used < MESH_SEGMENTS then
            used = used + 1
            local q = particles[b1j]
            local d = math.sqrt(b1d)
            modify(mesh_lines[used], {
                x1 = p.x, y1 = p.y, x2 = q.x, y2 = q.y,
                color = hsla(205 + t * 14 + d * 0.4, 70, 45, (1 - d / MESH_RADIUS) * 130),
                thickness = 1,
            })
        end
        if b2j > 0 and used < MESH_SEGMENTS then
            used = used + 1
            local q = particles[b2j]
            local d = math.sqrt(b2d)
            modify(mesh_lines[used], {
                x1 = p.x, y1 = p.y, x2 = q.x, y2 = q.y,
                color = hsla(225 + t * 14 + d * 0.4, 62, 42, (1 - d / MESH_RADIUS) * 95),
                thickness = 1,
            })
        end
    end

    for i = used + 1, mesh_used_prev do
        modify(mesh_lines[i], { color = HIDDEN })
    end
    mesh_used_prev = used

    return used
end

local function update_fractal(t)
    local max_depth = 3 + math.floor((t * 0.32) % 7)
    local idx = 1

    local function branch(x1, y1, angle, len, depth)
        if depth <= 0 or idx > FRACTAL_SEGMENTS then return end

        local sway = math.sin(t * 1.1 + depth * 0.55) * 5
        local rad = math.rad(angle + sway)
        local x2 = x1 + math.cos(rad) * len
        local y2 = y1 - math.sin(rad) * len

        local hue = (198 + depth * 16 + t * 16) % 360
        modify(fractal_lines[idx], {
            x1 = x1, y1 = y1,
            x2 = x2, y2 = y2,
            color = hsla(hue, 75, 46 + depth * 2, 220),
            thickness = clamp(depth * 0.45, 1, 4),
        })
        idx = idx + 1

        branch(x2, y2, angle - 28, len * 0.72, depth - 1)
        branch(x2, y2, angle + 28, len * 0.72, depth - 1)
    end

    branch(SIDE.x + SIDE.w / 2, SIDE.y + 218, 90, 62, max_depth)

    for i = idx, FRACTAL_SEGMENTS do
        modify(fractal_lines[i], { color = HIDDEN })
    end

    return max_depth
end

local function update_image(t)
    if not image_id then return end

    local next_index = (math.floor(t / 2.5) % #FILTERS) + 1
    if next_index ~= filter_index then
        filter_index = next_index
        modify(image_id, { filter = FILTERS[filter_index] })
        modify(hud.filter, { content = "FILTER: " .. FILTER_LABELS[filter_index] })
    end
end

local function update_hud(t, mesh_used, growth_depth)
    modify(hud.sub, {
        content = string.format(
            "mode=%s  seed=%d  source=%s  |  rotations: xw / yz / zw",
            OPT.mode, OPT.seed, dataset_label
        ),
    })
    modify(hud.mode, {
        content = string.format("SOURCE  %s", dataset_label:upper()),
    })
    modify(hud.growth, {
        content = string.format("L-SYSTEM GROWTH  %d / 9", growth_depth),
    })
    modify(hud.stats, {
        content = string.format(
            "t=%6.1fs   curve=%d   nodes=%d   links=%d",
            t, ATTRACTOR_SEGMENTS, PARTICLE_COUNT, mesh_used
        ),
    })
end

function update(dt)
    local t = time() * 0.001

    update_attractor(t)
    local mesh_used = update_flow(t, dt)
    local growth_depth = update_fractal(t)
    update_image(t)
    update_hud(t, mesh_used, growth_depth)
end

-- mesh.lua — flow-field particle mesh only, with the bottom stats strip.
--
-- Usage:
--   artc examples/mesh.lua
--   artc examples/mesh.lua -- seed=42 radius=72 nodes=600

window(1200, 800)
bg("#05060a")
fps(30)
palette("catppuccin")
seed(7)

local W, H = 1200, 800
local HIDDEN = "#05060a"
local TAU = math.pi * 2

local function clamp(v, lo, hi)
    if v < lo then return lo end
    if v > hi then return hi end
    return v
end

--------------------------------------------------------------------------------
-- CLI options
--------------------------------------------------------------------------------

local OPT = { seed = 7, radius = 64, nodes = 520 }

local function parse_args()
    local n = (type(script_argc) == "function") and script_argc() or 0
    for i = 1, n do
        local token = arg[i] or ""
        local key, value = token:match("^([%w_]+)=(.*)$")
        if key == "seed" then
            OPT.seed = tonumber(value) or OPT.seed
        elseif key == "radius" then
            OPT.radius = tonumber(value) or OPT.radius
        elseif key == "nodes" then
            OPT.nodes = tonumber(value) or OPT.nodes
        end
    end
end

parse_args()
seed(OPT.seed)

--------------------------------------------------------------------------------
-- Layout / entity budget
--------------------------------------------------------------------------------

local FIELD = { x = 16, y = 16, w = 1168, h = 720 }
local FIELD_CX = FIELD.x + FIELD.w / 2
local FIELD_CY = FIELD.y + FIELD.h / 2
local STATS_Y = 766

local PARTICLE_COUNT = clamp(math.floor(OPT.nodes), 20, 900)
local MESH_RADIUS = clamp(OPT.radius, 24, 120)
local MESH_SEGMENTS = PARTICLE_COUNT * 2
local CELL = MESH_RADIUS
local GRID_COLS = math.ceil(FIELD.w / CELL)
local GRID_ROWS = math.ceil(FIELD.h / CELL)

local particles = {}
local mesh_lines = {}
local stats
local mesh_used_prev = 0

--------------------------------------------------------------------------------
-- Helpers
--------------------------------------------------------------------------------

local function hsla(h, s, l, a)
    local rgb = color.to_rgb(color.hsl(h % 360, s, l))
    return color.rgba(rgb.r, rgb.g, rgb.b, clamp(math.floor(a), 0, 255))
end

local function grid_coords(p)
    return clamp(math.floor((p.x - FIELD.x) / CELL) + 1, 1, GRID_COLS),
           clamp(math.floor((p.y - FIELD.y) / CELL) + 1, 1, GRID_ROWS)
end

--------------------------------------------------------------------------------
-- Setup
--------------------------------------------------------------------------------

function setup()
    -- Bottom stats strip.
    rectangle({ x = 0, y = 752, w = W, h = 48, color = "#080b14", motion = "none" })
    line({ x1 = 0, y1 = 752, x2 = W, y2 = 752, color = "#243049", thickness = 1 })

    -- Mesh pool (created before nodes so links render underneath).
    for i = 1, MESH_SEGMENTS do
        mesh_lines[i] = line({ color = HIDDEN, thickness = 1 })
    end

    -- Flow particles.
    for i = 1, PARTICLE_COUNT do
        local p = {
            x = rand(FIELD.x + 6, FIELD.x + FIELD.w - 6),
            y = rand(FIELD.y + 6, FIELD.y + FIELD.h - 6),
            phase = rand(0, TAU),
            size = rand(2.0, 3.8),
            gx = 1,
            gy = 1,
        }
        p.id = circle({
            x = p.x,
            y = p.y,
            size = p.size,
            color = "#91d7e3",
            motion = "none",
        })
        particles[i] = p
    end

    stats = text({
        x = 20, y = STATS_Y,
        font = font.cozette, size = 15,
        fg = palette.light_grey, bg = "#080b14", blend = true,
        content = "",
    })
end

--------------------------------------------------------------------------------
-- Update
--------------------------------------------------------------------------------

local function update_flow(t, dt)
    local buckets = {}

    for i = 1, PARTICLE_COUNT do
        local p = particles[i]

        local fx = math.sin((p.y - FIELD_CY) * 0.016 + t * 0.7 + p.phase)
                 + 0.35 * math.cos((p.x - FIELD_CX) * 0.011 - t * 0.4)
        local fy = math.cos((p.x - FIELD_CX) * 0.014 - t * 0.6 + p.phase)
                 - 0.35 * math.sin((p.y - FIELD_CY) * 0.010 + t * 0.3)

        p.x = p.x + fx * 38 * dt
        p.y = p.y + fy * 38 * dt

        local margin = 4
        if p.x < FIELD.x + margin then
            p.x = FIELD.x + FIELD.w - margin
        elseif p.x > FIELD.x + FIELD.w - margin then
            p.x = FIELD.x + margin
        end
        if p.y < FIELD.y + margin then
            p.y = FIELD.y + FIELD.h - margin
        elseif p.y > FIELD.y + FIELD.h - margin then
            p.y = FIELD.y + margin
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
            size = p.size + mag * 0.7,
            color = hsla(hue, 85, light, 220),
        })
    end

    local used = 0
    local radius_sq = MESH_RADIUS * MESH_RADIUS

    for i = 1, PARTICLE_COUNT do
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
                color = hsla(205 + t * 14 + d * 0.4, 70, 45,
                             (1 - d / MESH_RADIUS) * 140),
                thickness = 1,
            })
        end

        if b2j > 0 and used < MESH_SEGMENTS then
            used = used + 1
            local q = particles[b2j]
            local d = math.sqrt(b2d)
            modify(mesh_lines[used], {
                x1 = p.x, y1 = p.y, x2 = q.x, y2 = q.y,
                color = hsla(225 + t * 14 + d * 0.4, 62, 42,
                             (1 - d / MESH_RADIUS) * 100),
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

function update(dt)
    local t = time() * 0.001
    local links = update_flow(t, dt)

    modify(stats, {
        content = string.format(
            "t=%6.1fs   nodes=%d   links=%d   radius=%d   field=curl-sine",
            t, PARTICLE_COUNT, links, MESH_RADIUS
        ),
    })
end

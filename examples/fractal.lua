-- https://lsa.umich.edu/content/dam/cscs-assets/cscs-documents/jingyig_REU_report.pdf

window(800, 600)
bg("#000000")
seed(42)
fps(30)

local max_depth = 10
local base_length = 150
local grow_rate = 1.0  -- seconds per depth level

function setup() end

local function draw_branch(x1, y1, angle, len, depth, t)
    if depth == 0 then return end

    local sway = math.sin(t + depth * 0.5) * 10
    local a = angle + sway
    local rad = math.rad(a)

    local x2 = x1 + math.cos(rad) * len
    local y2 = y1 - math.sin(rad) * len

    local hue = (120 + depth * 15) % 360
    local col = color.hsl(hue, 80, 60)

    line {
        x1 = x1,
        y1 = y1,
        x2 = x2,
        y2 = y2,
        color = col,
        thickness = depth
    }

    draw_branch(x2, y2, a - 30, len * 0.7, depth - 1, t)
    draw_branch(x2, y2, a + 30, len * 0.7, depth - 1, t)
end

function update(dt)
    local t = time() * 0.001
    clear()

    -- Calculate current depth based on elapsed time
    local current_depth = math.min(max_depth, math.floor(t / grow_rate))

    -- Draw tree with current growing depth
    draw_branch(400, 595, 90, base_length, current_depth, t)
end

-- Neon Recursive Fractal Arbors
window(800, 800)
palette("catppuccin")
bg(palette.black)
fps(60)

local lines = {}
local MAX_DEPTH = 8

-- Helper to track and update line entities
function create_or_update_line(index, x1, y1, x2, y2, depth)
    local color_val = color.hsl(180 + (depth * 20), 90, 60) -- Shifts from Cyan to Purple
    local thickness = MAX_DEPTH - depth + 1
    
    if lines[index] then
        modify(lines[index], { x1 = x1, y1 = y1, x2 = x2, y2 = y2, color = color_val })
    else
        lines[index] = line({
            x1 = x1, y1 = y1, 
            x2 = x2, y2 = y2, 
            color = color_val, 
            thickness = thickness 
        })
    end
end

local line_ptr = 1

function draw_branch(x, y, angle, length, depth, time_mod)
    if depth > MAX_DEPTH then return end

    -- Calculate end point
    local rad = angle * 0.0174533
    local nx = x + math.cos(rad) * length
    local ny = y + math.sin(rad) * length

    create_or_update_line(line_ptr, x, y, nx, ny, depth)
    line_ptr = line_ptr + 1

    -- Recursive step: Two branches
    -- The angle oscillates based on time for a "swaying" neon effect
    local bend = 20 + math.sin(time_mod + depth) * 15
    
    draw_branch(nx, ny, angle - bend, length * 0.75, depth + 1, time_mod)
    draw_branch(nx, ny, angle + bend, length * 0.75, depth + 1, time_mod)
end

function setup()
    -- Initial draw to populate the lines table
    update(0)
end

function update(dt)
    line_ptr = 1
    local t = time() / 1000
    
    -- Draw four fractal structures from the center for a kaleidoscopic effect
    for i = 0, 3 do
        local base_angle = (i * 90) + (math.sin(t * 0.2) * 10)
        draw_branch(400, 400, base_angle, 100, 1, t)
    end
end

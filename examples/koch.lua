-- Clean Colorful Koch Snowflake
window(800, 800)
palette("catppuccin")
bg(palette.black)
fps(60)

local lines = {}
local MAX_LINES = 4000 

function setup()
    for i = 1, MAX_LINES do
        -- Initialize lines as completely transparent
        lines[i] = line({width = 2, color = color.rgba(0,0,0,0)})
    end
end

local line_index = 1

function add_segments(x1, y1, x2, y2, depth, current_gen)
    if depth <= 0 or line_index > MAX_LINES then
        local l = lines[line_index]
        if l then
            -- Color shifts based on position in the snowflake and time
            local hue = (line_index * 0.15 + time() * 0.04) % 360
            local neon = color.hsl(hue, 95, 65)
            
            modify(l, {
                x1 = x1, y1 = y1, 
                x2 = x2, y2 = y2, 
                color = neon,
                width = math.max(1, 2.5 - current_gen * 0.3)
            })
            line_index = line_index + 1
        end
        return
    end

    local dx = (x2 - x1) / 3
    local dy = (y2 - y1) / 3

    local p1x, p1y = x1 + dx, y1 + dy
    local p3x, p3y = x1 + 2 * dx, y1 + 2 * dy

    -- Peak calculation (no rotation applied to the whole set)
    local p2x = p1x + (dx * 0.5 + dy * 0.866)
    local p2y = p1y + (dy * 0.5 - dx * 0.866)

    add_segments(x1, y1, p1x, p1y, depth - 1, current_gen)
    add_segments(p1x, p1y, p2x, p2y, depth - 1, current_gen)
    add_segments(p2x, p2y, p3x, p3y, depth - 1, current_gen)
    add_segments(p3x, p3y, x2, y2, depth - 1, current_gen)
end

function update(dt)
    -- Cycle through generations 0 to 5
    local gen_speed = time() / 2500
    local current_gen = math.floor(gen_speed % 6)
    
    -- Hide all lines before redrawing the current generation
    for i = 1, MAX_LINES do 
        modify(lines[i], {color = palette.black}) 
    end
    
    line_index = 1
    local size = 520
    local h = size * (math.sqrt(3) / 2)
    local cx, cy = 400, 400
    
    -- Static Vertices
    local v1 = { x = cx, y = cy - h * (2/3) }
    local v2 = { x = cx - size/2, y = cy + h * (1/3) }
    local v3 = { x = cx + size/2, y = cy + h * (1/3) }

    -- Draw the three main branches of the snowflake
    add_segments(v1.x, v1.y, v3.x, v3.y, current_gen, current_gen)
    add_segments(v3.x, v3.y, v2.x, v2.y, current_gen, current_gen)
    add_segments(v2.x, v2.y, v1.x, v1.y, current_gen, current_gen)
end

window(800, 800)
fps(30)
bg("#fcfcfc")

local colors = {[0] = "#ff0000", "#00ff00", "#0000ff"}
local moving_lines = {}
local circle_lines = {}

function setup()
    local cx, radius = 400, 150
    local count = 100

    -- Store references to colorful vertical lines
    for i = 0, count - 1 do
        local x = cx - radius + (i * (2 * radius / count))
        local dx = x - cx
        local dy = math.sqrt(math.max(0, radius^2 - dx^2))
        local offset = (i % 2 ~= 0) and 0 or 5

        local id = line({
            color = colors[i % 3],
            x1 = x, y1 = 400 - dy - offset,
            x2 = x, y2 = 400 + dy - offset,
            thickness = 3
        })
        table.insert(circle_lines, {id = id, base_x = x, base_dy = dy, color_index = i % 3})
    end

    -- Black rectangle lines that move horizontally and vertically
    for i = 0, 30 do
        local x = 250 + i * 6
        local id = line({
            color = "#000000",
            x1 = x, y1 = 200,
            x2 = x, y2 = 600,
            thickness = 3
        })
        table.insert(moving_lines, id)
    end
end

function update(dt)
    local t = time() * 0.001

    local x_offset = math.sin(t * 1.2) * 50
    local y_offset = math.cos(t * 1.2) * 20

    for i = 0, #moving_lines - 1 do
        local x = 250 + i * 10 + x_offset
        modify(moving_lines[i + 1], {
            x1 = x, y1 = 200 + y_offset,
            x2 = x, y2 = 600 + y_offset
        })
    end

    for i = 1, #circle_lines do
        local line_data = circle_lines[i]
        local pulse = math.sin(t * 3 + i * 0.1) * 20
        local dy = line_data.base_dy + pulse
        local new_color_index = (line_data.color_index + math.floor(t)) % 3
        modify(line_data.id, {
            y1 = 400 - dy,
            y2 = 400 + dy,
            color = colors[new_color_index]
        })
    end
end

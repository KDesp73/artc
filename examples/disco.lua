local scale = 2
window(400 * scale, 400 * scale)
bg(color.rgb(20, 20, 20))
seed(73)
fps(30)

local size = 20 * scale
local spacing = 20 * scale
local width, height = size, size

local ids = {}
local positions = {}

function setup()
    for i = 0, width - 1 do
        for j = 0, height - 1 do
            local x = 10 * scale + i * spacing
            local y = 10 * scale + j * spacing

            local id = circle({
                x = x,
                y = y,
                color = "#ffffff",
                size = 8
            })
            table.insert(ids, id)
            table.insert(positions, {x = x, y = y})
        end
    end
end

function update(dt)
    local t = time() * 0.001

    for i, id in ipairs(ids) do
        local base = positions[i]
        local wave = math.sin(t + i * 0.15) * 5
        local wiggle = math.cos(t * 2 + i * 0.05) * 2

        local offsetX = math.sin(t * 1.5 + i * 0.1) * 4
        local offsetY = math.cos(t + i * 0.2) * 4

        local h = (time() * 20 + i * 0.5) % 360
        local s = 80 + math.sin(t * 3 + i) * 20
        local l = 50 + math.sin(t * 2 + i * 0.5) * 30

        local sizePulse = 6 + math.sin(t * 4 + i) * 2

        modify(id, {
            x = base.x + offsetX,
            y = base.y + offsetY + wave + wiggle,
            size = sizePulse,
            color = color.hsl(h, s, l)
        })
    end
end

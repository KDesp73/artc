window(700, 550)
bg("#000000")
seed(42)
fps(30)

local cols, rows = 10, 8
local spacing = 60
local startX, startY = 80, 60
local grid = {}

function random_shape()
    local shapes = { "circle", "square", "triangle" }
    return shapes[math.random(#shapes)]
end

function spawn(x, y)
    return {
        id = object({
            type = random_shape(),
            x = x,
            y = y,
            size = 20,
            color = "#ffffff",
            motion = "none",
        }),
        x = x,
        y = y,
        t = 0,
        type = "circle"
    }
end

function setup()
    for r = 1, rows do
        for c = 1, cols do
            local x = startX + (c - 1) * spacing
            local y = startY + (r - 1) * spacing
            table.insert(grid, spawn(x, y))
        end
    end
end

function update(dt)
    local time_now = time() * 0.001

    for i, cell in ipairs(grid) do
        cell.t = cell.t + dt

        -- Every 2 seconds, change the shape
        if cell.t > 2 then
            cell.t = 0
            local new_kind = random_shape()
            cell.kind = new_kind
            modify(cell.id, { type = new_kind })
        end

        -- Pulse size and cycle color smoothly
        local s = 10 + math.sin(time_now + i * 0.5) * 8
        local r = 127 + math.sin(time_now + i)     * 127
        local g = 127 + math.sin(time_now + i + 2) * 127
        local b = 127 + math.sin(time_now + i + 4) * 127


        modify(cell.id, {
            size = s,
            color = hex({ r = r, g = g, b = b }),
        })
    end
end


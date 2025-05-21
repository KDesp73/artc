window(640, 480)
bg("#000000")
fps(60)

local ids = {}

function setup()
    for i = 1, 100 do
        local id = circle({
            x = 0,
            y = 0,
            size = 10,
            color = "#000000",
            motion = "none",
            speed = 0,
            radius = 20
        })
        table.insert(ids, id)
    end
end

function update(dt)
    local t = time() * 0.001

    for i = 1, #ids do
        local offset = i * 0.2
        local x = 320 + math.sin(t + offset) * 200
        local y = 240 + math.cos(t + i * 0.3) * 200

        local r = 127 + math.sin(t + i)     * 127
        local g = 127 + math.sin(t + i + 2) * 127
        local b = 127 + math.sin(t + i + 4) * 127

        modify(ids[i], {
            x = x,
            y = y,
            color = hex({ r = r, g = g, b = b })
        })
    end
end


window(1080, 720)
bg("#000000")
seed(73)

function setup()
    local cx = 1080 / 2
    local cy = 720 / 2
    local radius = 200      -- Distance from center
    local count = 20        -- Number of circles
    local size = 20

    for i = 1, count do
        local angle = (i / count) * (2 * math.pi)
        local x = cx + math.cos(angle) * radius
        local y = cy + math.sin(angle) * radius

        local r = 127 + math.sin(i)     * 127
        local g = 127 + math.sin(i + 2) * 127
        local b = 127 + math.sin(i + 4) * 127

        circle({
            x = x,
            y = y,
            size = size,
            color = hex({ r = r, g = g, b = b }),
            motion = "pulse",
            radius = 20 + i * 3/2,
            speed = 2,
        })
    end
end

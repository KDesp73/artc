window(800, 600)
bg("#000014")

local elements = {}
local count = 100

function setup()
    for i = 1, count do
        local x = math.random(100, 700)
        local y = math.random(100, 500)
        local angle = math.random() * 2 * math.pi
        local len = 40
        local speed = 40 + math.random() * 40
        local dir = math.random() * 2 * math.pi
        local rot_speed = (math.random() - 0.5) * 2

        local dx = math.cos(angle) * len / 2
        local dy = math.sin(angle) * len / 2

        local dot1 = circle({
            x = x + dx, y = y + dy, size = 4,
            color = "#ffffff", motion = "none", speed = 0
        })

        local dot2 = circle({
            x = x - dx, y = y - dy, size = 4,
            color = "#ffffff", motion = "none", speed = 0
        })

        local line_id = line({
            x1 = x + dx, y1 = y + dy,
            x2 = x - dx, y2 = y - dy,
            color = "#88ccff"
        })

        table.insert(elements, {
            x = x, y = y,
            angle = angle, rot_speed = rot_speed,
            len = len, speed = speed, dir = dir,
            dots = { dot1, dot2 }, line = line_id
        })
    end
end

function update(dt)
    for _, e in ipairs(elements) do
        e.x = e.x + math.cos(e.dir) * e.speed * dt
        e.y = e.y + math.sin(e.dir) * e.speed * dt

        -- Bounce off window edges
        if e.x < 20 or e.x > 780 then e.dir = math.pi - e.dir end
        if e.y < 20 or e.y > 580 then e.dir = -e.dir end

        -- Rotate
        e.angle = e.angle + e.rot_speed * dt

        local dx = math.cos(e.angle) * e.len / 2
        local dy = math.sin(e.angle) * e.len / 2

        local x1 = e.x + dx
        local y1 = e.y + dy
        local x2 = e.x - dx
        local y2 = e.y - dy

        modify(e.dots[1], { x = x1, y = y1 })
        modify(e.dots[2], { x = x2, y = y2 })
        modify(e.line, { x1 = x1, y1 = y1, x2 = x2, y2 = y2 })
    end
end


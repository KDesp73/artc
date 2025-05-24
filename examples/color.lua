window(800, 800)
bg(color.rgb(0, 0, 14))

local centerX, centerY = 400, 400
local rings = 12
local segments = 36
local radius = 150
local shapes = {}

function setup()
    for r = 1, rings do
        for s = 1, segments do
            local angle = (s / segments) * math.pi * 2
            local dist = (r / rings) * radius
            local x = centerX + math.cos(angle) * dist
            local y = centerY + math.sin(angle) * dist

            local hue = (s / segments) * 360
            local hexColor = color.hsl(hue, 100, 50)

            local id = circle({
                x = x,
                y = y,
                size = 8,
                color = hexColor,
                motion = "none",
                speed = 0,
            })

            table.insert(shapes, {
                id = id,
                baseHue = hue,
                r = r,
                s = s,
                dist = dist,
                angle = angle
            })
        end
    end
end

function update(dt)
    local t = time() * 0.001

    for _, s in ipairs(shapes) do
        local hue = (s.baseHue + t * 30 + s.r * 15) % 360
        local hexColor = color.hsl(hue, 100, 50)

        local angleOffset = t * 0.4
        local angle = s.angle + angleOffset
        local x = centerX + math.cos(angle) * s.dist
        local y = centerY + math.sin(angle) * s.dist

        modify(s.id, {
            x = x,
            y = y,
            color = hexColor
        })
    end
end

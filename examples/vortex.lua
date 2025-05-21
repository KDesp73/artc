window(800, 800)
bg("#000014")
seed(42)

local particles = {}
local count = 200
local centerX, centerY = 400, 400

function setup()
    for i = 1, count do
        local angle = (i / count) * math.pi * 2
        local radius = math.random(50, 300)
        local size = math.random(4, 12)
        local speed = 0.5 + math.random() * 1.5

        local id = circle({
            x = centerX + math.cos(angle) * radius,
            y = centerY + math.sin(angle) * radius,
            size = size,
            color = "#ffffff",
            motion = "none",
            speed = 0
        })

        particles[i] = {
            id = id,
            angle = angle,
            radius = radius,
            size = size,
            speed = speed,
            offset = math.random() * 2 * math.pi
        }
    end
end

function hslToRgb(h, s, l)
    -- Based on https://stackoverflow.com/a/9493060
    if s == 0 then
        local gray = l * 255
        return gray, gray, gray
    else
        local function hue2rgb(p, q, t)
            if t < 0 then t = t + 1 end
            if t > 1 then t = t - 1 end
            if t < 1/6 then return p + (q - p) * 6 * t end
            if t < 1/2 then return q end
            if t < 2/3 then return p + (q - p) * (2/3 - t) * 6 end
            return p
        end

        local q = l < 0.5 and l * (1 + s) or l + s - l * s
        local p = 2 * l - q
        local r = hue2rgb(p, q, h + 1/3)
        local g = hue2rgb(p, q, h)
        local b = hue2rgb(p, q, h - 1/3)
        return r * 255, g * 255, b * 255
    end
end

function update(dt)
    local t = time() * 0.001

    for i = 1, #particles do
        local p = particles[i]
        local a = p.angle + t * p.speed
        local r = p.radius + math.sin(t * 0.5 + i) * 10

        local x = centerX + math.cos(a + p.offset) * r
        local y = centerY + math.sin(a + p.offset) * r

        -- Create smooth hue cycling
        local hue = (t * 0.05 + i / count) % 1.0
        local lightness = 0.4 + 0.3 * math.sin(t * 2 + i)
        local r, g, b = hslToRgb(hue, 1.0, lightness)

        modify(p.id, {
            x = x,
            y = y,
            color = hex({ r = r, g = g, b = b }),
            size = p.size + math.sin(t + i) * 1.5
        })
    end
end


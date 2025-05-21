window(800, 800)
bg("#101010")
seed(1234)

local grid = {}
local cols, rows = 20, 20
local spacing = 40
local startX = 15
local startY = 15

function setup()
    for i = 0, cols - 1 do
        for j = 0, rows - 1 do
            local x = startX + i * spacing
            local y = startY + j * spacing
            local id = square({
                x = x,
                y = y,
                angle = 0,
                color = "#ffffff",
                motion = "none"
            })

            table.insert(grid, {
                id = id,
                gx = i,
                gy = j,
                baseX = x,
                baseY = y
            })
        end
    end
end

function update(dt)
    local t = time() * 0.001

    for i = 1, #grid do
        local cell = grid[i]
        local gx = cell.gx
        local gy = cell.gy

        -- Animate size based on distance from center + time
        local dx = gx - cols / 2
        local dy = gy - rows / 2
        local dist = math.sqrt(dx * dx + dy * dy)

        local pulse = math.sin(t * 2 + dist * 0.5)
        local size = 8 + pulse * 8
        local angle = math.sin(t + gx * 0.2 + gy * 0.3) * math.pi

        local hue = ((gx + gy) / (cols + rows) + t * 0.1) % 1.0
        local r, g, b = hslToRgb(hue, 1.0, 0.6)

        modify(cell.id, {
            w = size,
            h = size,
            angle = angle,
            color = hex({ r = r, g = g, b = b })
        })
    end
end

function hslToRgb(h, s, l)
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


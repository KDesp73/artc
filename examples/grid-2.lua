window(800, 800)
bg("#101010")
seed(2025)

local grid = {}
local cols, rows = 20, 20
local spacing = 40
local startX = 15
local startY = 15

-- Precompute grid center
local centerX = (cols - 1) / 2
local centerY = (rows - 1) / 2

function setup()
    for i = 0, cols - 1 do
        for j = 0, rows - 1 do
            local x = startX + i * spacing
            local y = startY + j * spacing
            local id = square({
                x = x,
                y = y,
                size = 10,
                angle = 0,
                color = "#ffffff",
                motion = "none"
            })

            table.insert(grid, {
                id = id,
                gx = i,
                gy = j,
                baseX = x,
                baseY = y,
                dx = i - centerX,
                dy = j - centerY
            })
        end
    end
end

function update(dt)
    local t = time() * 0.001

    for i = 1, #grid do
        local cell = grid[i]

        -- Distance from grid center
        local dist = math.sqrt(cell.dx^2 + cell.dy^2)
        local wave = math.sin(dist * 0.5 - t * 2)
        local size = 10 + wave * 15

        local angle = wave * 0.5
        local hue = ((dist / (cols * 0.5)) + t * 0.1) % 1.0
        local r, g, b = hslToRgb(hue, 1.0, 0.55 + 0.2 * wave)

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


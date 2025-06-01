local scale = 2
window(520 * scale, 520 * scale)
palette("catppuccin")
bg(palette.black)
fps(30)

local glyphs = { " ", ".", ":", "-", "=", "+", "*", "#", "%", "@" }

local grid = {}
local cols, rows = 20, 20
local cell_size = 24 * scale

-- Calculate grid size for centering
local grid_width = cols * cell_size
local grid_height = rows * cell_size

function setup()
    local f = font.cozette
    for y = 1, rows do
        for x = 1, cols do
            local tx = text({
                x       = x * cell_size,
                y       = y * cell_size,
                font    = f,
                size    = cell_size,
                fg      = palette.purple,
                bg      = palette.black,
                content = glyphs[1],
            })
            table.insert(grid, {
                obj    = tx,
                col    = x,
                row    = y,
                phaseX = (x / cols) * math.pi * 2,
                phaseY = (y / rows) * math.pi * 2,
            })
        end
    end
end

function update(dt)
    local t = time() / 1000

    for _, cell in ipairs(grid) do
        -- New waving pattern: combine sine and cosine for a ripple effect
        local wave = (math.sin(t * 5 + cell.phaseX * 3) + math.cos(t * 4 + cell.phaseY * 4)) / 2

        -- Normalize wave to [0,1]
        local norm = (wave + 1) / 2

        -- Select glyph index based on wave
        local idx = math.floor(norm * (#glyphs - 1) + 0.5) + 1

        -- Color cycling: hue changes over time and position for rainbow flow
        local baseHue = ((t * 100) + cell.col * 10 + cell.row * 5) % 360
        local saturation = 85
        local lightness = 60 + 15 * math.sin(t * 10 + cell.col + cell.row) -- subtle lightness pulsation

        local fg = color.hsl(baseHue, saturation, lightness)

        modify(cell.obj, {
            content = glyphs[idx],
            fg = fg,
        })
    end
end

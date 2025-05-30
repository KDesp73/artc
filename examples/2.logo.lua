local scale = 2  -- Increased scale for bigger glyphs
local letter_width = 5
local letter_height = 7
local spacing = 1

local letter_a = {
    "     ",
    " ####",
    "#   #",
    "#   #",
    "#  ##",
    " ## #",
    "     ",
}

local letter_r = {
    "     ",
    "#### ",
    "#   #",
    "#    ",
    "#    ",
    "#    ",
    "     ",
}

local letter_t = {
    " #   ",
    "#### ",
    " #   ",
    " #   ",
    " #   ",
    "  ###",
    "     ",
}

local letter_c = {
    "     ",
    " ### ",
    "#   #",
    "#    ",
    "#   #",
    " ### ",
    "     ",
}

local letters = {letter_a, letter_r, letter_t, letter_c}

local total_width = (#letters * letter_width) + ((#letters - 1) * spacing)
local total_height = letter_height

local cell_size = 20 * scale

local rows = 20  -- fewer rows, bigger glyphs
local width, height = 600 * scale, 400 * scale

window(width, height)

palette("catppuccin")
bg(palette.black)
fps(15)

-- Build combined bitmap
local artc_bitmap = {}
for y = 1, total_height do
    artc_bitmap[y] = {}
    for x = 1, total_width do
        artc_bitmap[y][x] = false
    end
end

for i, letter in ipairs(letters) do
    local offset_x = (i - 1) * (letter_width + spacing)
    for y = 1, letter_height do
        local row = letter[y]
        for x = 1, letter_width do
            if row:sub(x,x) == "#" then
                artc_bitmap[y][offset_x + x] = true
            end
        end
    end
end

-- Calculate columns based on width
local cols = math.floor(width / cell_size)

-- Offsets to center bitmap in grid
local offset_x = math.floor((cols - total_width) / 2)
local offset_y = math.floor((rows - total_height) / 2)

local glyphs = { " ", ".", ":", "-", "=", "+", "*", "#", "%", "@" }
local grid = {}

function setup()
    local f = font.cozette
    for y = 1, rows do
        for x = 1, cols do
            local bx = x - offset_x
            local by = y - offset_y
            local inside = false
            if bx >= 1 and bx <= total_width and by >= 1 and by <= total_height then
                inside = artc_bitmap[by][bx]
            end

            local tx = text({
                x       = (x - 1) * cell_size,
                y       = (y - 1) * cell_size,
                font    = f,
                size    = cell_size,
                fg      = palette.purple,
                bg      = palette.black,
                content = glyphs[1],
                blend   = true,
            })

            table.insert(grid, {
                obj    = tx,
                col    = x,
                row    = y,
                inside = inside,
                phaseX = (x / cols) * math.pi * 2,
                phaseY = (y / rows) * math.pi * 2,
            })
        end
    end
end

function update(dt)
    local t = time() * 0.0025

    for _, cell in ipairs(grid) do
        local wave = (math.sin(t * 5 + cell.phaseX * 3) + math.cos(t * 4 + cell.phaseY * 4)) / 2
        local norm = (wave + 1) / 2

        local idx

        if cell.inside then
            -- Inside: white color and higher-density glyph
            idx = math.floor(norm * (#glyphs - 1) + 0.5) + 1
            if idx < #glyphs then idx = idx + 2 end
            if idx > #glyphs then idx = #glyphs end
            if idx < 1 then idx = 1 end

            modify(cell.obj, {
                content = glyphs[idx],
                fg = palette.light_blue,
            })
        else
            -- Outside: animated colors and sparser glyphs
            idx = math.floor(norm * (#glyphs / 3)) + 1
            if idx > #glyphs then idx = #glyphs end
            if idx < 1 then idx = 1 end

            local baseHue = ((t * 100) + cell.col * 10 + cell.row * 5) % 360
            local saturation = 40
            local lightness = 30

            local fg = color.hsl(baseHue, saturation, lightness)

            modify(cell.obj, {
                content = glyphs[idx],
                fg = fg,
            })
        end
    end
end

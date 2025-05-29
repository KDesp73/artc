window(800, 800)
palette("catppuccin")
bg(palette.black)

local base_size = 16 * 4
local color_shift = 0
local text1, text2

function setup()
    text1 = text({
        x = 100, y = 200,
        align = "center",
        font = "artc:funky",
        size = base_size,
        fg = palette.yellow,
        bg = palette.black,
        content = "Hello World"
    })

    text2 = text({
        x = 400, y = 500,
        align = "center",
        font = "artc:cozette",
        size = base_size,
        fg = palette.green,
        bg = palette.black,
        content = "Minecraft"
    })
end

function update(dt)
    local t = time() * 0.001

    -- Pulse effect (rounded size)
    local scale = 1 + 0.25 * math.sin(t * 2)
    local pulsing_size = math.floor(base_size * scale + 0.5)
    modify(text1, {
        size = pulsing_size
    })

    -- Rotate color hue over time for text2
    color_shift = (color_shift + dt * 20) % 360
    local rainbow = color.hsl(color_shift, 80, 60)
    modify(text2, { fg = rainbow })
end

local downscale = 4
local width  = math.floor(708 / downscale)
local height = math.floor(885 / downscale)
local screen_width = width * 4
local screen_height = height * 2

window(screen_width, screen_height)

local src = "https://www.rollingstone.com/wp-content/uploads/2024/08/R1391_Random-Notes_Hozier_A-e1723646684359.jpg"

function setup()
    local count = 0
    image({
        x = 0, y = 0,
        w = width, h = height,
        src = src,
    })
    count = count + 1
    image({
        x = width * count, y = 0,
        w = width, h = height,
        src = src,
        filter = filter.channel_swap
    })
    count = count + 1
    image({
        x = width * count, y = 0,
        w = width, h = height,
        src = src,
        filter = filter.greyscale
    })
    count = count + 1
    image({
        x = width * count, y = 0,
        w = width, h = height,
        src = src,
        filter = filter.negative
    })
    count = 0
    image({
        x = width * count, y = height,
        w = width, h = height,
        src = src,
        filter = filter.sepia
    })
    count = count + 1
    image({
        x = width * count, y = height,
        w = width, h = height,
        src = src,
        filter = filter.invert_red
    })
    count = count + 1
    image({
        x = width * count, y = height,
        w = width, h = height,
        src = src,
        filter = filter.invert_green
    })
    count = count + 1
    image({
        x = width * count, y = height,
        w = width, h = height,
        src = src,
        filter = filter.invert_blue
    })
    count = count + 1

end


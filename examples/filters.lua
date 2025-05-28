local downscale = 4
local width  = math.floor(856 / downscale)
local height = math.floor(951 / downscale)
local screen_width = width * 4
local screen_height = height * 2

window(screen_width, screen_height)

local src = "./examples/grandma.jpg"

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
        filter = "channel-swap"
    })
    count = count + 1
    image({
        x = width * count, y = 0,
        w = width, h = height,
        src = src,
        filter = "greyscale"
    })
    count = count + 1
    image({
        x = width * count, y = 0,
        w = width, h = height,
        src = src,
        filter = "negative"
    })
    count = 0
    image({
        x = width * count, y = height,
        w = width, h = height,
        src = src,
        filter = "sepia"
    })
    count = count + 1
    image({
        x = width * count, y = height,
        w = width, h = height,
        src = src,
        filter = "invert-red"
    })
    count = count + 1
    image({
        x = width * count, y = height,
        w = width, h = height,
        src = src,
        filter = "invert-green"
    })
    count = count + 1
    image({
        x = width * count, y = height,
        w = width, h = height,
        src = src,
        filter = "invert-blue"
    })
    count = count + 1

end

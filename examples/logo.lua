local scale = 2
local width, height = 600 * scale, 400 * scale
window(width, height)
palette("catppuccin")
bg(palette.black)
fps(30)

local entities = {}

function setup()
    -- Ambient pulsing circles
    for i = 1, 20 do
        local x = rand(0, width)
        local y = rand(0, height)
        local size = rand(30, 80)
        local color = color.hsl(rand(0, 360), 60, 50)
        table.insert(entities, {
            id = circle({x = x, y = y, size = size, color = color}),
            base_size = size,
            pulse_speed = rand(1, 3)
        })
    end

    -- Bouncing triangle
    entities.bounce = triangle({
        x = width / 2,
        y = height / 4,
        size = 50 * scale,
        color = palette.purple,
        motion = "bounce",
        speed = 2,
        radius = 20
    })

    -- Animated background grid lines
    for i = 1, width, 40 do
        table.insert(entities, {
            id = line({x1 = i, y1 = 0, x2 = i, y2 = height, color = palette.grey, thickness = 1}),
            axis = "x", speed = rand(10, 30),
            base_x = i
        })
    end
    for j = 1, height, 40 do
        table.insert(entities, {
            id = line({x1 = 0, y1 = j, x2 = width, y2 = j, color = palette.grey, thickness = 1}),
            axis = "y", speed = rand(10, 30),
            base_y = j
        })
    end

    local font_size = 250 * scale
    text({
        x = (width - (font_size * 1.9)) / 2,
        y = (height - font_size) / 2,
        font = font.cozette,
        size = font_size,
        content = "artc",
        bg = palette.black,
        fg = palette.light_blue,
        blend = true
    })
end

function update(dt)
    local t = time() / 1000

    -- Animate pulsing circles
    for _, e in ipairs(entities) do
        if e.base_size then
            local pulse = math.sin(t * e.pulse_speed) * 10
            modify(e.id, {size = e.base_size + pulse})

        elseif e.axis == "x" and e.base_x then
            local shift = math.sin(t / e.speed) * 10
            modify(e.id, {x1 = e.base_x + shift, x2 = e.base_x + shift})

        elseif e.axis == "y" and e.base_y then
            local shift = math.cos(t / e.speed) * 10
            modify(e.id, {y1 = e.base_y + shift, y2 = e.base_y + shift})
        end
    end

    -- Animate triangle color
    local hue = (t * 40) % 360
    local tri_color = color.hsl(hue, 80, 60)
    modify(entities.bounce, {color = tri_color})
end

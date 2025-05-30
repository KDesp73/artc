local scale = 2
local width, height = 600 * scale, 400 * scale
window(width, height)
palette("catppuccin")
bg(palette.black)
fps(30)

local particles = {}
local font_size = 250 * scale
local text_x = (width - (font_size * 1.9)) / 2
local text_y = (height - font_size) / 2
local text_w = font_size * 2
local text_h = font_size

function setup()
    -- Center text
    text({
        x = text_x,
        y = text_y,
        font = font.cozette,
        size = font_size,
        content = "artc",
        bg = palette.black,
        fg = palette.light_blue,
        blend = true
    })
end

function spawn_particle()
    local px = rand(text_x, text_x + text_w)
    local py = rand(text_y, text_y + text_h)
    local angle = rand(0, 360)
    local speed = rand(10, 30)
    local dx = math.cos(math.rad(angle)) * speed
    local dy = math.sin(math.rad(angle)) * speed
    local size = rand(2, 5)
    local hue = rand(180, 220)
    local id = circle({x = px, y = py, size = size, color = color.hsl(hue, 70, 60), blend = true})

    table.insert(particles, {
        id = id,
        x = px, y = py,
        dx = dx, dy = dy,
        size = size,
        age = 0,
        lifespan = rand(1.5, 3.0),
        hue = hue
    })
end

function update(dt)
    -- Spawn particles over time
    if math.random() < dt * 10 then
        spawn_particle()
    end

    -- Animate particles
    for i = #particles, 1, -1 do
        local p = particles[i]
        p.age = p.age + dt
        if p.age >= p.lifespan then
            table.remove(particles, i)
        else
            local fade = 1 - (p.age / p.lifespan)
            local lightness = 60 * fade + 10  -- fade to darker
            local saturation = 70 * fade      -- fade to grey
            p.x = p.x + p.dx * dt
            p.y = p.y + p.dy * dt
            modify(p.id, {
                x = p.x,
                y = p.y,
                size = p.size * fade,
                color = color.hsl(p.hue, saturation, lightness)
            })
        end
    end
end

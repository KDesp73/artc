window(800, 800)
bg("#ffffff")
fps(30)

local id
local x, y = 0, 0
local w, h = 150, 90

-- derived so that it hits corners perfectly
local drawable_w = 800 - w
local drawable_h = 800 - h

local speed = 300
local vx = speed
local vy = speed * (drawable_h / drawable_w)

function setup()
    id = image({
        x = x, y = y,
        w = w, h = h,
        src = "./examples/dvd.png",
    })
end

function update(dt)
    x = x + vx * dt
    y = y + vy * dt

    -- bounce off left/right edges
    if x < 0 then
        x = 0
        vx = -vx
    elseif x > drawable_w then
        x = drawable_w
        vx = -vx
    end

    -- bounce off top/bottom edges
    if y < 0 then
        y = 0
        vy = -vy
    elseif y > drawable_h then
        y = drawable_h
        vy = -vy
    end

    modify(id, { x = x, y = y })
end


window(800, 800)
bg("#000000")
fps(30)

local id
local x, y = 400, 400
local w, h = 150, 90
local vx, vy = 200, 150

function setup()
    id = image({
        x = x, y = y,
        w = w, h = h,
        src = "./examples/dvd.png",
        filter = "negative"
    })
end

function update(dt)
    x = x + vx * dt
    y = y + vy * dt

    -- bounce off left/right edges
    if x < 0 then
        x = 0
        vx = -vx
    elseif x > 800 - w then
        x = 800 - w
        vx = -vx
    end

    -- bounce off top/bottom edges
    if y < 0 then
        y = 0
        vy = -vy
    elseif y > 800 - h then
        y = 800 - h
        vy = -vy
    end

    modify(id, { x = x, y = y })
end

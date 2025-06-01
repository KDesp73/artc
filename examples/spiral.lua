window(800, 800)
bg("#000014")
fps(30)

local centerX, centerY = 400, 400
local points = {}
local lines = {}
local depth = 200
local rings = 40
local segments = 20
local spacing = 20
local speed = 60

function setup()
    for i = 1, rings do
        local z = i * spacing
        for j = 1, segments do
            local angle = (j / segments) * math.pi * 2
            local x = math.cos(angle) * 100
            local y = math.sin(angle) * 100

            table.insert(points, {
                x = x, y = y, z = z,
                id = circle({
                    x = 0, y = 0, size = 2,
                    color = "#ffffff", motion = "none", speed = 0
                })
            })
        end
    end

    for i = 1, #points - segments do
        if (i % segments ~= 0) then
            table.insert(lines, {
                a = i,
                b = i + 1,
                id = line({ x1 = 0, y1 = 0, x2 = 0, y2 = 0, color = "#8888ff" })
            })
        end
        table.insert(lines, {
            a = i,
            b = i + segments,
            id = line({ x1 = 0, y1 = 0, x2 = 0, y2 = 0, color = "#4444ff" })
        })
    end
end

local function project(x, y, z)
    local scale = 400 / (z + 1)
    return centerX + x * scale, centerY + y * scale
end

function update(dt)
    local t = time() * 0.001
    local positions = {}

    for i, p in ipairs(points) do
        p.z = p.z - dt * speed
        if p.z < 1 then
            p.z = rings * spacing
        end

        local angle = t + i * 0.02
        local spiralRadius = 100 + 20 * math.sin(t + i * 0.1)
        local x = math.cos(angle) * spiralRadius
        local y = math.sin(angle) * spiralRadius
        local px, py = project(x, y, p.z)

        modify(p.id, { x = px, y = py })
        positions[i] = { x = px, y = py }
    end

    for _, l in ipairs(lines) do
        local a = positions[l.a]
        local b = positions[l.b]
        if a and b then
            modify(l.id, {
                x1 = a.x, y1 = a.y,
                x2 = b.x, y2 = b.y
            })
        end
    end
end


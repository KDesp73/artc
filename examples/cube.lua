window(800, 800)
bg("#000014")

local centerX, centerY = 400, 400
local cube = {}
local lines = {}
local size = 100

function setup()
    local function addPoint(x, y, z)
        return {
            x = x, y = y, z = z,
            id = circle({
                x = 0, y = 0, size = 4,
                color = "#ffffff", motion = "none", speed = 0
            })
        }
    end

    local s = size
    -- Define 8 cube vertices
    cube = {
        addPoint(-s, -s, -s), addPoint(s, -s, -s),
        addPoint(s, s, -s), addPoint(-s, s, -s),
        addPoint(-s, -s, s), addPoint(s, -s, s),
        addPoint(s, s, s), addPoint(-s, s, s),
    }

    local edges = {
        {1,2},{2,3},{3,4},{4,1}, -- back face
        {5,6},{6,7},{7,8},{8,5}, -- front face
        {1,5},{2,6},{3,7},{4,8}  -- connections
    }

    -- Draw edges as lines
    for _, edge in ipairs(edges) do
        table.insert(lines, {
            a = edge[1], b = edge[2],
            id = line({
                x1 = 0, y1 = 0, x2 = 0, y2 = 0,
                color = "#8888ff"
            })
        })
    end
end

-- 3D rotation function
local function rotate(point, t)
    local x, y, z = point.x, point.y, point.z

    -- Rotate around Y axis
    local sinY, cosY = math.sin(t), math.cos(t)
    x, z = x * cosY - z * sinY, x * sinY + z * cosY

    -- Rotate around X axis
    local sinX, cosX = math.sin(t * 0.6), math.cos(t * 0.6)
    y, z = y * cosX - z * sinX, y * sinX + z * cosX

    return x, y, z
end

-- Perspective projection
local function project(x, y, z)
    local scale = 400 / (z + 400)
    return centerX + x * scale, centerY + y * scale
end

function update(dt)
    local t = time() * 0.001

    local positions = {}

    for i, p in ipairs(cube) do
        local x, y, z = rotate(p, t)
        local px, py = project(x, y, z)
        modify(p.id, { x = px, y = py })
        positions[i] = { x = px, y = py }
    end

    for _, l in ipairs(lines) do
        local a = positions[l.a]
        local b = positions[l.b]
        modify(l.id, { x1 = a.x, y1 = a.y, x2 = b.x, y2 = b.y })
    end
end


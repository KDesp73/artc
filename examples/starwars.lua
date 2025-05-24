window(800, 800)
bg("#000014")
seed(73)

local centerX, centerY = 400, 400
local beams = {}
local grid_size = 12
local spacing = 60
local speed = 120

function setup()
    for i = -grid_size, grid_size do
        for j = -grid_size, grid_size do
            for k = 1, 6 do
                local z = k * spacing + math.random() * 30
                table.insert(beams, {
                    x = i * spacing * 0.5 + math.random(-10,10),
                    y = j * spacing * 0.5 + math.random(-10,10),
                    z = z,
                    len = 15 + math.random() * 10,
                    id = line({
                        x1 = 0, y1 = 0,
                        x2 = 0, y2 = 0,
                        color = "#44ccff"
                    })
                })
            end
        end
    end
end

local function project(x, y, z)
    local fov = 400
    local scale = fov / (z + 1)
    return centerX + x * scale, centerY + y * scale
end

function update(dt)
    for _, beam in ipairs(beams) do
        beam.z = beam.z - dt * speed
        if beam.z < 1 then
            beam.z = spacing * 6 + rand() * 30
        end

        local px, py = project(beam.x, beam.y, beam.z)
        local px2, py2 = project(beam.x, beam.y, beam.z + beam.len)

        local brightness = 1 - (beam.z / (spacing * 6))
        local alpha = math.floor(brightness * 255)
        local color = string.format("#44ccff%02x", alpha)

        modify(beam.id, {
            x1 = px, y1 = py,
            x2 = px2, y2 = py2,
            color = color
        })
    end
end


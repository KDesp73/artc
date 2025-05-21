-- color.lua
local color = {}

-- Clamp helper
local function clamp(x, min, max)
    return math.max(min, math.min(max, x))
end

-- Convert RGB to hex string (e.g., "#ff00aa")
function color.rgb_to_hex(r, g, b)
    return string.format("#%02x%02x%02x", clamp(r, 0, 255), clamp(g, 0, 255), clamp(b, 0, 255))
end

-- Convert hex string to RGB
function color.hex_to_rgb(hex)
    hex = hex:gsub("#", "")
    if #hex == 3 then
        return tonumber("0x"..hex:sub(1,1):rep(2)),
        tonumber("0x"..hex:sub(2,2):rep(2)),
        tonumber("0x"..hex:sub(3,3):rep(2))
    elseif #hex == 6 then
        return tonumber("0x"..hex:sub(1,2)),
        tonumber("0x"..hex:sub(3,4)),
        tonumber("0x"..hex:sub(5,6))
    else
        error("Invalid hex format: " .. hex)
    end
end

-- Convert RGB to HSV
function color.rgb_to_hsv(r, g, b)
    r, g, b = r/255, g/255, b/255
    local max = math.max(r, g, b)
    local min = math.min(r, g, b)
    local delta = max - min

    local h, s, v
    v = max

    if delta == 0 then
        h = 0
    elseif max == r then
        h = 60 * (((g - b) / delta) % 6)
    elseif max == g then
        h = 60 * (((b - r) / delta) + 2)
    else
        h = 60 * (((r - g) / delta) + 4)
    end

    if max == 0 then
        s = 0
    else
        s = delta / max
    end

    return h, s, v
end

-- Convert HSV to RGB
function color.hsv_to_rgb(h, s, v)
    local c = v * s
    local x = c * (1 - math.abs((h / 60) % 2 - 1))
    local m = v - c

    local r, g, b
    if h < 60 then r, g, b = c, x, 0
    elseif h < 120 then r, g, b = x, c, 0
    elseif h < 180 then r, g, b = 0, c, x
    elseif h < 240 then r, g, b = 0, x, c
    elseif h < 300 then r, g, b = x, 0, c
    else r, g, b = c, 0, x
    end

    return math.floor((r + m) * 255),
    math.floor((g + m) * 255),
    math.floor((b + m) * 255)
end

-- Linear interpolation between two RGB colors
function color.lerp_color(c1, c2, t)
    local r = math.floor(c1[1] + (c2[1] - c1[1]) * t)
    local g = math.floor(c1[2] + (c2[2] - c1[2]) * t)
    local b = math.floor(c1[3] + (c2[3] - c1[3]) * t)
    return {r, g, b}
end

-- Darken color by a factor (0.0 - 1.0)
function color.darken(r, g, b, factor)
    return math.floor(r * factor),
    math.floor(g * factor),
    math.floor(b * factor)
end

-- Brighten color by a factor (0.0 - 1.0)
function color.brighten(r, g, b, factor)
    return clamp(math.floor(r + (255 - r) * factor), 0, 255),
    clamp(math.floor(g + (255 - g) * factor), 0, 255),
    clamp(math.floor(b + (255 - b) * factor), 0, 255)
end

return color

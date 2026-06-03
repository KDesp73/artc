-- y = A · cos(2π f_m t + φ) · sin(2π f_c t)

window(1000, 600)
bg("#0a0a0a")
fps(60)

local W, H = 1000, 600
local cx, cy = W * 0.5, H * 0.5
local tau = math.pi * 2

local A = 1.0
local F_C = 11.0
local F_M = 1.0
local F_M_DEV = 0.06
-- π/2 shifts the envelope so two full lobes sit at t = 0.25 and 0.75 (centered in [0, 1])
local PHI_M = math.pi * 0.5
local T_MIN, T_MAX = 0, 1
local SEGMENTS = 800

local curve_lines = {}
local axis_x_id, axis_y_id, label_id

local function y_at(t, f_m)
    return A * math.cos(tau * f_m * t + PHI_M) * math.sin(tau * F_C * t)
end

local function map_x(t)
    return cx + (t - (T_MIN + T_MAX) * 0.5) * (W * 0.84 / (T_MAX - T_MIN))
end

local function map_y(y)
    return cy - y * (H * 0.38)
end

local function create_or_update_line(index, x1, y1, x2, y2, col, thickness)
    if curve_lines[index] then
        modify(curve_lines[index], {
            x1 = x1, y1 = y1,
            x2 = x2, y2 = y2,
            color = col,
            thickness = thickness,
        })
    else
        curve_lines[index] = line({
            x1 = x1, y1 = y1,
            x2 = x2, y2 = y2,
            color = col,
            thickness = thickness,
        })
    end
end

function setup()
    axis_x_id = line({
        x1 = map_x(T_MIN), y1 = cy,
        x2 = map_x(T_MAX), y2 = cy,
        color = color.rgba(255, 255, 255, 24),
        thickness = 1,
    })

    axis_y_id = line({
        x1 = cx, y1 = map_y(-1.15),
        x2 = cx, y2 = map_y(1.15),
        color = color.rgba(255, 255, 255, 24),
        thickness = 1,
    })

    label_id = text({
        x = 24,
        y = 24,
        content = "",
        font = font.cozette,
        size = 20,
        fg = "#aaaaaa",
        bg = "#0a0a0a",
        blend = true,
    })

    update(0)
end

function update(dt)
    local now = time() * 0.001
    local f_m = F_M + F_M_DEV * math.sin(now * 0.7)

    local prev_x, prev_y
    local line_idx = 1

    for i = 1, SEGMENTS do
        local t = T_MIN + (i - 1) / (SEGMENTS - 1) * (T_MAX - T_MIN)
        local y = y_at(t, f_m)
        local px, py = map_x(t), map_y(y)

        if prev_x then
            create_or_update_line(line_idx, prev_x, prev_y, px, py, "#ffffff", 2)
            line_idx = line_idx + 1
        end

        prev_x, prev_y = px, py
    end

    modify(label_id, {
        content = string.format(
            "y(t) = A cos(2 pi f_m t + pi/2) sin(2 pi f_c t)   A = %.1f   f_c = %.1f Hz   f_m = %.3f Hz",
            A, F_C, f_m
        ),
    })
end

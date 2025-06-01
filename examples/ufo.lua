window(800, 600)
palette("catppuccin")
bg(palette.black)
fps(60)  -- Higher FPS for smoother flicker

local dome, beam, body, cow
local flicker_timer = 0
local base_cx = 400
local base_cy = 100
local time_accum = 0

-- Phase offset for cow float (90 degrees out of phase)
local cow_phase_offset = math.pi / 2  -- adjust to change phase difference

-- Fluorescent lamp properties
local beam_hue = 50       -- Yellow hue
local beam_saturation = 80 -- Slightly desaturated
local min_lightness = 30  -- Minimum brightness (dimmer moments)
local max_lightness = 95  -- Maximum brightness
local target_lightness = 80 -- Normal brightness
local current_lightness = target_lightness
local instability = 0     -- Current flicker intensity

function setup()
    -- text = text({
    --     x = 20, y = 20,
    --     content = "X-Files",
    --     font = font.xfiles,
    --     size = 36,
    --     blend = true
    -- })

    dome = ellipse({
        x = base_cx,
        y = base_cy,
        w = 60,
        h = 40,
        color = palette.light_blue
    })

    local beam_y = base_cy + 40
    beam = triangle({
        x = base_cx,
        y = beam_y,
        w = 300,
        h = 500,
        color = color.hsl(beam_hue, beam_saturation, target_lightness)
    })

    body = ellipse({
        x = base_cx,
        y = base_cy + 23,
        w = 130,
        h = 50,
        color = palette.light_grey
    })

    cow = image({
        x = base_cx - 50/2,
        y = beam_y + 200,
        w = 50, h = 50,
        src = "./examples/cow.png",
    })
end

function update(dt)
    time_accum = time_accum + dt

    local freq = 2 * math.pi / 6  -- frequency for floating motion (6 seconds period)

    -- UFO vertical offset
    local y_offset = math.sin(time_accum * freq) * 20

    -- Cow vertical offset with phase shift
    local beam_y = base_cy + 40
    local cow_y_offset = math.sin(time_accum * freq + cow_phase_offset) * 30

    -- Update UFO parts vertical positions
    modify(dome, { y = base_cy + y_offset })
    modify(beam, { y = base_cy + 40 + y_offset })
    modify(body, { y = base_cy + 23 + y_offset })

    -- Update cow vertical position separately
    modify(cow, { y = beam_y + 200 + cow_y_offset })

    -- Fluorescent lamp flicker logic
    flicker_timer = flicker_timer + dt

    -- Base flicker (constant small variations)
    local base_flicker = (math.random() * 6 - 3)  -- -3 to +3

    -- Occasionally add instability (bigger flickers)
    if math.random() < 0.02 then  -- 2% chance per frame
        instability = math.random() * 30  -- Up to 30 units of instability
    else
        instability = instability * 0.9  -- Decay instability
    end

    -- Combine effects
    current_lightness = target_lightness + base_flicker + (math.random() * instability)

    -- Clamp to min/max values
    current_lightness = math.max(min_lightness, math.min(max_lightness, current_lightness))

    -- Apply the flicker to beam and silhouettes
    local beam_color = color.hsl(beam_hue, beam_saturation, current_lightness)
    modify(beam, { color = beam_color })

    -- Occasionally add a "stutter" effect (rapid on/off)
    if flicker_timer > 0.5 and math.random() < 0.1 then
        flicker_timer = 0
        local stutter_count = math.random(2, 4)
        for _ = 1, stutter_count do
            modify(beam, { color = color.hsl(beam_hue, beam_saturation, min_lightness) })
            modify(beam, { color = beam_color })
        end
    end
end

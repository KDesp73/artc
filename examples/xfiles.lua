-- xfiles.lua — an X-Files-inspired abduction surveillance visualization.
--
--   artc examples/xfiles.lua
--
-- Uses the embedded xfiles fonts, a flickering saucer beam, a greyscale cow,
-- CRT scanlines, signal static, and a green dossier HUD.

window(1200, 800)
bg("#020705")
fps(30)
seed(1993)
palette("catppuccin")

local W, H = 1200, 800
local CX = 600
local TAU = math.pi * 2
local HIDDEN = "#020705"
local COW_SRC = "./examples/cow.png"

local function clamp(v, lo, hi)
    if v < lo then return lo end
    if v > hi then return hi end
    return v
end

local function hsla(h, s, l, a)
    local rgb = color.to_rgb(color.hsl(h % 360, s, l))
    return color.rgba(rgb.r, rgb.g, rgb.b, clamp(math.floor(a), 0, 255))
end

local function border(x, y, w, h, col)
    line({ x1 = x, y1 = y, x2 = x + w, y2 = y, color = col, thickness = 1 })
    line({ x1 = x, y1 = y + h, x2 = x + w, y2 = y + h, color = col, thickness = 1 })
    line({ x1 = x, y1 = y, x2 = x, y2 = y + h, color = col, thickness = 1 })
    line({ x1 = x + w, y1 = y, x2 = x + w, y2 = y + h, color = col, thickness = 1 })
end

--------------------------------------------------------------------------------
-- State
--------------------------------------------------------------------------------

local ufo = {}
local lights = {}
local beam = {}
local stars = {}
local flecks = {}
local hud = {}
local cow_id = nil
local cow_filter_index = 1
local beam_instability = 0

local COW_FILTERS = { filter.greyscale, filter.threshold }
local LOG_LINES = {
    "02:14  SUNKEN ROAD",
    "02:17  LOW FREQUENCY HUM",
    "02:19  MISSING LIVESTOCK",
    "02:23  ELECTROMAGNETIC SPIKE",
}

--------------------------------------------------------------------------------
-- Setup
--------------------------------------------------------------------------------

function setup()
    -- Dossier panels.
    rectangle({ x = 36, y = 110, w = 260, h = 600, color = "#03100b", motion = "none" })
    rectangle({ x = 904, y = 110, w = 260, h = 600, color = "#03100b", motion = "none" })
    border(36, 110, 260, 600, "#1e5b3f")
    border(904, 110, 260, 600, "#1e5b3f")

    -- Header / footer rules.
    line({ x1 = 0, y1 = 92, x2 = W, y2 = 92, color = "#1e5b3f", thickness = 1 })
    rectangle({ x = 0, y = 756, w = W, h = 44, color = "#03100b", motion = "none" })
    line({ x1 = 0, y1 = 756, x2 = W, y2 = 756, color = "#1e5b3f", thickness = 1 })

    -- Faint X sigil behind the scene.
    for i = 1, 6 do
        local off = i * 5
        local col = color.rgb(8 + i * 4, 28 + i * 12, 18 + i * 8)
        local thick = 7 - i
        line({
            x1 = CX - 250 - off, y1 = 128,
            x2 = CX + 250 + off, y2 = 704,
            color = col, thickness = thick,
        })
        line({
            x1 = CX + 250 + off, y1 = 128,
            x2 = CX - 250 - off, y2 = 704,
            color = col, thickness = thick,
        })
    end

    -- Perspective ground grid.
    for i = 0, 12 do
        local y = 520 + i * i * 1.65
        line({
            x1 = 320, y1 = y, x2 = 880, y2 = y,
            color = "#0b2418", thickness = 1,
        })
    end
    for i = -8, 8 do
        line({
            x1 = CX, y1 = 520,
            x2 = CX + i * 76, y2 = 736,
            color = "#0b2418", thickness = 1,
        })
    end

    -- Distant stars / fireflies.
    for i = 1, 90 do
        local s = {
            x = rand(320, 880),
            y = rand(120, 700),
            phase = rand(0, TAU),
        }
        s.id = circle({
            x = s.x, y = s.y,
            size = rand(0.8, 2.0),
            color = "#173d2a",
            motion = "none",
        })
        stars[i] = s
    end

    -- Beam layers, drawn before the saucer and cow.
    beam.outer = triangle({ x = CX, y = 190, w = 360, h = 520, color = "#0d2b1f", motion = "none" })
    beam.mid   = triangle({ x = CX, y = 190, w = 230, h = 500, color = "#174934", motion = "none" })
    beam.core  = triangle({ x = CX, y = 190, w = 100, h = 470, color = "#256847", motion = "none" })

    -- Abducted cow.
    if type(file_exists) == "function" and file_exists(COW_SRC) then
        cow_id = image({
            x = CX - 34, y = 420,
            w = 68, h = 68,
            src = COW_SRC,
            filter = COW_FILTERS[cow_filter_index],
            motion = "none",
        })
    end

    -- Saucer.
    ufo.rim  = ellipse({ x = CX, y = 216, w = 184, h = 26, color = "#31564a", motion = "none" })
    ufo.body = ellipse({ x = CX, y = 205, w = 158, h = 46, color = "#9fb8ae", motion = "none" })
    ufo.dome = ellipse({ x = CX, y = 178, w = 72, h = 40, color = "#91d7e3", motion = "none" })

    for i = 1, 7 do
        lights[i] = circle({
            x = CX + (i - 4) * 22,
            y = 224,
            size = 4,
            color = "#eed49f",
            motion = "none",
        })
    end

    -- CRT scanline overlay.
    for y = 0, H, 8 do
        line({ x1 = 0, y1 = y, x2 = W, y2 = y, color = "#06130d", thickness = 1 })
    end

    -- Signal static flecks.
    for i = 1, 56 do
        local f = {
            x = rand(320, 880),
            y = rand(100, 740),
            phase = rand(0, TAU),
        }
        f.id = square({ x = f.x, y = f.y, size = 2, color = HIDDEN, motion = "none" })
        flecks[i] = f
    end

    -- Text, above the CRT overlay.
    hud.title = text({
        x = 342, y = 26,
        font = font.xfiles, size = 32,
        fg = "#8fe0b0", bg = "#020705", blend = true,
        content = "THE TRUTH IS OUT THERE",
    })
    hud.believe = text({
        x = 438, y = 712,
        font = font.xfiles_1, size = 26,
        fg = "#6fbf8e", bg = "#020705", blend = true,
        content = "I WANT TO BELIEVE",
    })

    hud.case_tag = text({
        x = 52, y = 130,
        font = font.cozette, size = 18,
        fg = "#a8f0c2", bg = "#03100b", blend = true,
        content = "X-FILES // CASE 001",
    })
    hud.classification = text({
        x = 52, y = 162,
        font = font.cozette, size = 13,
        fg = "#e8b4a0", bg = "#03100b", blend = true,
        content = "CLASSIFICATION: RESTRICTED",
    })
    hud.subject = {
        text({
            x = 52, y = 198,
            font = font.cozette, size = 14,
            fg = "#6fbf8e", bg = "#03100b", blend = true,
            content = "SUBJECT: UNIDENTIFIED",
        }),
        text({
            x = 52, y = 216,
            font = font.cozette, size = 14,
            fg = "#6fbf8e", bg = "#03100b", blend = true,
            content = "AERIAL PHENOMENON",
        }),
    }

    local notes = {
        "FIELD NOTES:",
        "- NO THERMAL SIGNATURE",
        "- LOW-FREQ HUM 42 Hz",
        "- LIVESTOCK MISSING",
        "- WITNESS RELIABLE?",
    }
    hud.fieldnotes = {}
    for i, note in ipairs(notes) do
        hud.fieldnotes[i] = text({
            x = 52, y = 260 + (i - 1) * 20,
            font = font.cozette, size = 13,
            fg = "#75a98a", bg = "#03100b", blend = true,
            content = note,
        })
    end
    hud.sigil = text({
        x = 96, y = 560,
        font = font.xfiles, size = 72,
        fg = "#234b34", bg = "#03100b", blend = true,
        content = "X",
    })

    hud.evidence = text({
        x = 920, y = 130,
        font = font.cozette, size = 17,
        fg = "#a8f0c2", bg = "#03100b", blend = true,
        content = "EVIDENCE LOG",
    })
    hud.log = {}
    for i, label in ipairs(LOG_LINES) do
        hud.log[i] = text({
            x = 920, y = 170 + (i - 1) * 30,
            font = font.cozette, size = 13,
            fg = "#4e8f6d", bg = "#03100b", blend = true,
            content = label,
        })
    end
    hud.counter = text({
        x = 920, y = 330,
        font = font.cozette, size = 13,
        fg = "#75a98a", bg = "#03100b", blend = true,
        content = "",
    })
    hud.poster = text({
        x = 936, y = 610,
        font = font.xfiles_1, size = 20,
        fg = "#6fbf8e", bg = "#03100b", blend = true,
        content = "I WANT TO BELIEVE",
    })

    hud.status = text({
        x = 40, y = 768,
        font = font.cozette, size = 15,
        fg = palette.light_grey, bg = "#03100b", blend = true,
        content = "",
    })
end

--------------------------------------------------------------------------------
-- Update
--------------------------------------------------------------------------------

function update(dt)
    local t = time() * 0.001

    -- Slow saucer drift.
    local ux = CX + math.sin(t * 0.37) * 22
    local uy = 176 + math.sin(t * 0.9) * 14

    modify(ufo.dome, {
        x = ux, y = uy,
        w = 72 + math.sin(t * 2.1) * 4,
        h = 40,
    })
    modify(ufo.body, { x = ux, y = uy + 27, w = 158, h = 46 })
    modify(ufo.rim,  { x = ux, y = uy + 38, w = 184, h = 26 })

    -- Beam flicker with occasional instability spikes.
    if rand(0, 1) < 0.02 then beam_instability = rand(0, 20) end
    beam_instability = beam_instability * 0.92

    local bright = 30 + math.sin(t * 9) * 4 + rand(0, beam_instability)
    local by = uy + 46

    modify(beam.outer, {
        x = ux, y = by,
        w = 360 + math.sin(t * 3.1) * 8,
        color = color.hsl(155, 60, clamp(bright - 16, 8, 45)),
    })
    modify(beam.mid, {
        x = ux, y = by,
        w = 230 + math.sin(t * 4.3) * 6,
        color = color.hsl(150, 65, clamp(bright - 8, 12, 55)),
    })
    modify(beam.core, {
        x = ux, y = by,
        w = 100 + math.sin(t * 5.7) * 6,
        color = color.hsl(145, 70, clamp(bright, 18, 70)),
    })

    -- Cow abduction float and surveillance filter cycling.
    if cow_id then
        local cow_y = uy + 252 + math.sin(t * 0.9 + math.pi / 2) * 22
        local cow_x = ux + math.sin(t * 0.5) * 8
        local cow_size = 68 + math.sin(t * 3) * 3

        modify(cow_id, {
            x = cow_x - cow_size / 2,
            y = cow_y - cow_size / 2,
            w = cow_size,
            h = cow_size,
        })

        local next_filter = (math.floor(t / 3) % #COW_FILTERS) + 1
        if next_filter ~= cow_filter_index then
            cow_filter_index = next_filter
            modify(cow_id, { filter = COW_FILTERS[cow_filter_index] })
        end
    end

    -- Running lights.
    for i = 1, #lights do
        local pulse = 3 + math.sin(t * 7 + i * 0.9) * 1.2 + rand(0, beam_instability * 0.08)
        modify(lights[i], {
            x = ux + (i - 4) * 22,
            y = uy + 48,
            size = pulse,
            color = hsla(50 + i * 4, 85, 72, 240),
        })
    end

    -- Twinkling stars.
    for i = 1, #stars do
        local s = stars[i]
        local l = 10 + math.sin(t * 1.6 + s.phase) * 6
        modify(s.id, {
            color = color.hsl(145, 55, clamp(l, 5, 24)),
            size = 1.2 + math.sin(t * 2 + s.phase) * 0.6,
        })
    end

    -- Random signal static.
    for i = 1, #flecks do
        local f = flecks[i]
        if rand(0, 1) < 0.28 then
            f.x = rand(320, 880)
            f.y = rand(100, 740)
        end
        local on = math.sin(t * 13 + f.phase) > 0.55
        modify(f.id, {
            x = f.x,
            y = f.y,
            size = on and rand(1, 3) or 0,
            color = on and "#9fe8c0" or HIDDEN,
        })
    end

    -- HUD activity.
    local active = (math.floor(t / 2) % #LOG_LINES) + 1
    for i = 1, #hud.log do
        modify(hud.log[i], {
            fg = (i == active) and "#b7f5cf" or "#4e8f6d",
        })
    end

    local believe_col = (math.sin(t * 1.7) > 0.94) and "#d8ffe6" or "#6fbf8e"
    modify(hud.believe, { fg = believe_col })
    modify(hud.poster,  { fg = believe_col })

    modify(hud.counter, {
        content = string.format("SIGNAL EVENTS: %04d  //  %02d:%02d",
            math.floor(t * 17) % 10000,
            math.floor(t / 60) % 100,
            math.floor(t) % 60
        ),
    })
    modify(hud.status, {
        content = string.format(
            "CASE X-1993-001   //   t=%6.1fs   //   signal=%.1f   //   subject=BOVINE   //   STATUS: ACTIVE",
            t, 42 + beam_instability
        ),
    })
end

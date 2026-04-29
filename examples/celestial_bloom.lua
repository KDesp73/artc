-- Celestial Bloom
-- Layered generative piece: golden-angle phyllotaxis, ion drift, rotating rays, and pulsing halos.
-- Run: artc ./examples/celestial_bloom.lua

window(900, 900)
palette("catppuccin")
bg(palette.black)
seed(2026)
fps(60)

local W, H = 900, 900
local cx, cy = W * 0.5, H * 0.5
local tau = math.pi * 2
local golden = math.pi * (3 - math.sqrt(5))

-- Tunable density (keep total entities well under engine max for smooth playback)
local BLOOM_N = 300
local ION_N = 200
local RAY_N = 56
local HALO_N = 6

local bloom = {}
local ions = {}
local rays = {}
local halos = {}
local title_id

local function wrap_hue(h)
    h = h % 360
    if h < 0 then h = h + 360 end
    return h
end

function setup()
    -- A) Phyllotaxis cloud: each petal follows a Fermat spiral
    for i = 1, BLOOM_N do
        local a = i * golden
        local rad = 5.2 * math.sqrt(i)
        local sz = 2.2 + (i % 9) * 0.35 + math.min(3, i / 80)
        local hue0 = wrap_hue(210 + (i % 90) * 1.2)
        local id = circle({
            x = cx,
            y = cy,
            size = sz,
            color = color.hsl(hue0, 78, 52),
            motion = "none",
            speed = 0,
        })
        bloom[i] = {
            id = id,
            a0 = a,
            r0 = rad,
            i = i,
        }
    end

    -- B) Ion field: dust in a thick annulus, each with its own slow epicycle
    for j = 1, ION_N do
        local ang = rand(0, 360) * (math.pi / 180)
        local dist = 95 + rand(0, 1) * 310
        local sz = 1.4 + (j % 5) * 0.45
        local h = wrap_hue(300 + (j % 50) * 1.4)
        local id = circle({
            x = cx + math.cos(ang) * dist * 0.4,
            y = cy + math.sin(ang) * dist * 0.4,
            size = sz,
            color = color.hsl(h, 88, 62),
            motion = "none",
            speed = 0,
        })
        ions[j] = {
            id = id,
            base_ang = ang,
            dist = dist,
            w = 0.12 + (j % 7) * 0.02,
            ph = j * 0.19,
        }
    end

    -- C) Soft light rays (thin lines = fast renderer path)
    for r = 1, RAY_N do
        local id = line({
            x1 = cx,
            y1 = cy,
            x2 = cx + 120,
            y2 = cy,
            color = color.hsl(265, 28, 38),
            thickness = 0,
        })
        rays[r] = { id = id, a0 = (r - 1) / RAY_N * tau }
    end

    -- D) Concentric mist halos
    for h = 1, HALO_N do
        local bw = 32 + h * 38
        local bh = 24 + h * 30
        local li = 18 + h * 3
        local id = ellipse({
            x = cx,
            y = cy,
            w = bw,
            h = bh,
            color = color.hsl(275, 45, li),
            motion = "none",
            speed = 0,
        })
        halos[h] = { id = id, base_w = bw, base_h = bh, ph = h * 0.85 }
    end

    -- E) Quiet caption (optional; comment out to save one entity)
    title_id = text({
        x = 28,
        y = H - 40,
        content = "celestial bloom  ·  artc",
        font = font.cozette,
        size = 18,
        fg = palette.light_grey,
        bg = palette.black,
        blend = true,
    })
end

function update(dt)
    local t = time() * 0.001
    local slow = t * 0.35
    local pulse = 0.5 + 0.5 * math.sin(t * 1.1)

    -- Bloom: rotate, breathe radius, size shimmer, iridescent hue
    for i = 1, BLOOM_N do
        local p = bloom[i]
        local a = p.a0 + slow * (0.6 + 0.15 * math.sin(t * 0.2 + p.i * 0.01))
        local r = p.r0 * (1.0 + 0.04 * math.sin(t * 0.9 + p.i * 0.07))
        local x = cx + math.cos(a) * r
        local y = cy + math.sin(a) * r
        local h = wrap_hue(200 + t * 12 + p.i * 0.35 + 8 * math.sin(t * 0.4 + p.a0))
        local s = 72 + 8 * math.sin(t * 0.7 + p.i * 0.02)
        local l = 42 + 14 * math.sin(t * 1.3 + p.i * 0.04) * pulse
        local sz = 2.2 + (p.i % 9) * 0.35 + math.min(3, p.i / 80) + 0.6 * math.sin(t * 2.1 + p.i * 0.1)
        modify(p.id, {
            x = x,
            y = y,
            size = sz,
            color = color.hsl(h, s, l),
        })
    end

    -- Ions: slow orbit + wobble; warmer hues near the rim
    for j = 1, ION_N do
        local p = ions[j]
        local ang = p.base_ang + t * 0.22 * p.w + 0.18 * math.sin(t * 0.5 + p.ph)
        local wob = 1.0 + 0.12 * math.sin(t * 1.2 + p.ph) + 0.06 * math.sin(t * 2.0 - j * 0.03)
        local d = p.dist * wob
        local x = cx + math.cos(ang) * d * 0.92
        local y = cy + math.sin(ang) * d * 0.88
        local edge = d / 400
        local h = wrap_hue(285 + edge * 40 + t * 6 + 10 * math.sin(t * 0.3 + j * 0.1))
        local s = 85 - edge * 15
        local l = 55 + 12 * math.sin(t * 0.8 + p.ph) - edge * 8
        local sz = 1.4 + (j % 5) * 0.45 + 0.4 * math.sin(t * 1.4 + j * 0.2)
        modify(p.id, {
            x = x,
            y = y,
            size = sz,
            color = color.hsl(h, s, l),
        })
    end

    -- Rays: sweep, breathe length, subtle hue drift
    for r = 1, RAY_N do
        local p = rays[r]
        local ang = p.a0 + t * 0.18 + 0.04 * math.sin(t * 0.3 + r)
        local len = 95 + 130 * (0.55 + 0.45 * math.sin(t * 0.6 + r * 0.2) ^ 2)
        local h = wrap_hue(250 + r * 2.2 + t * 4)
        local x2 = cx + math.cos(ang) * len
        local y2 = cy + math.sin(ang) * len
        modify(p.id, {
            x1 = cx,
            y1 = cy,
            x2 = x2,
            y2 = y2,
            color = color.hsl(h, 32, 32 + 12 * math.sin(t + r * 0.1)),
        })
    end

    -- Halos: soft breathing, slight hue drift
    for h = 1, HALO_N do
        local p = halos[h]
        local b = 1.0 + 0.07 * math.sin(t * 0.45 + p.ph) + 0.03 * math.sin(t * 1.1 - h * 0.2)
        local hue = wrap_hue(270 + h * 5 + t * 3)
        local light = 16 + h * 2.5 + 3 * math.sin(t * 0.7 + p.ph)
        modify(p.id, {
            w = p.base_w * b,
            h = p.base_h * b,
            color = color.hsl(hue, 42, light),
        })
    end

    -- Subtle title shimmer
    if title_id then
        local g = 62 + 8 * math.sin(t * 0.5)
        modify(title_id, {
            fg = color.hsl(230, 12, g),
        })
    end
end

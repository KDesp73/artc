## ✅ TODO: API Expansion

### 🔲 Rectangle Primitive

Add support for rectangles:

```lua
rect(100, 100, 80, 40)
```

---

### 🔁 Custom Motions

Allow users to define their own motion paths:

```lua
function custom_path(t)
  return math.sin(t) * 100, math.cos(t) * 100
end

circle(300, 300, 20)
  :move(custom_path)
```

---

### 🔷 High-Level Primitives

#### Spiral

```lua
spiral(300, 300, 120, 3, 100)
```

#### Wave Line

```lua
wave_line(100, 300, 500, 300, 10, 20)
```

#### Grid

```lua
grid(5, 5, 20, function(x, y)
  circle(x, y, 10):color("sky")
end)
```

---

### 🧠 Built-in Modifiers

#### Pulse

```lua
circle(200, 200, 40):pulse(2)
```

#### Orbit

```lua
circle(0, 0, 10):orbit(200, 200, 100, 1)
```

#### Wiggle

```lua
rectangle(300, 300, 60, 30):wiggle(10, 2)
```

---

### 🎛️ Pattern & Palette System

#### Patterns

```lua
pattern("hexgrid", spacing = 30, radius = 12)
```

#### Palettes

```lua
apply_palette("pastel")
```

---

### 🧩 Composable Objects with Tags or Groups

```lua
g = group()
for i = 1, 10 do
  g:add(circle(50 * i, 200, 20):color("random"))
end

g:rotate(math.pi / 4)
g:pulse(1)
```

---

### 🎨 Presets

```lua
preset("kaleidoscope", {segments = 6})
preset("fireworks")
preset("lava_lamp")
```

## ✅ TODO: API Expansion

### Fonts / Typography

```lua
font("./font.ttf")

char({
    x = 10, y = 10,
    size = 20,
    content = 'C',
    font = "./another_font.ttf"
})

text({
    x = 20, y = 20,
    size = 10,
    content = "Hello World",
})

```

### Images

```lua
image({
    x = 10, y = 200,
    w = 100, h = 100,
    src = "./image.png",
    filter = "greyscale"
})
```

### Rectangle Primitive

Add support for rectangles:

```lua
rect({
    x = 100, y = 100, 
    w = 80, h = 40,
    color = "#ff00ff",
    motion = "none"
})
```

---

### Custom Motions

Allow users to define their own motion paths:

```lua
function custom_path(t)
  return math.sin(t) * 100, math.cos(t) * 100
end

circle({x = 300, y = 300, size = 20})
  :move(custom_path)
```

---

### High-Level Primitives

#### Spiral

```lua
spiral({300, 300, 120, 3, 100})
```

#### Wave Line

```lua
wave_line({100, 300, 500, 300, 10, 20})
```

#### Grid

```lua
grid(5, 5, 20, function(x, y)
  circle(x, y, 10):color("sky")
end)
```

---

### Composable Objects with Tags or Groups

```lua
g = group()
for i = 1, 10 do
  g:add(circle(50 * i, 200, 20):color("random"))
end

g:rotate(math.pi / 4)
g:pulse(1)
```

---

### Presets

```lua
preset("kaleidoscope", {segments = 6})
preset("fireworks")
preset("lava_lamp")
```

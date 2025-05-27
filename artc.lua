--- Artc Lua API
--- Written by KDesp73

--- Set background color of the scene
-- @param hex string
function bg(hex) end

--- Set scene dimensions
-- @param width number
-- @param height number
function window(width, height) end

--- Create a shape
-- @param opts table: {x, y, size, type, color, motion, speed, radius}
-- @return entity id
function shape(opts) end

--- Create a circle shape
-- @param opts table: {x, y, size, color, motion, speed, radius}
-- @return entity id
function circle(opts) end

--- Create a square shape
-- @param opts table: {x, y, size, color, motion, speed, radius}
-- @return entity id
function square(opts) end

--- Create a triangle shape
-- @param opts table: {x, y, size, color, motion, speed, radius}
-- @return entity id
function triangle(opts) end

--- Create a line entity
-- @param opts table: {x1, y1, x2, y2, color, thickness}
-- @return entity id
function line(opts) end

--- Create an image entity
-- @param opts table: {x, y, w, h, filter, src, motion, radius, speed}
-- @return entity id
function image(opts) end

--- Get elapsed time in milliseconds
-- @return number
function time() return 0 end

--- Generate a random number in range [min, max]
-- @param min number
-- @param max number
-- @return number
function rand(min, max) return 0 end

--- Convert rgb object into hex string
-- @param rgb table {r, g, b}
-- @return string
function hex(rgb) return "" end

--- Clears the scene
function clear() end

--- Set the seed
-- @param seed number
function seed(seed) end

--- Set the fps
-- @param fps number
function fps(fps) end

--- Modify an existing entity through its ID
-- @param id number
-- @param props table: depends on the type of entity
function modify(id, props) end

--- @class palette
palette = {
    white = "",
    black = "",
    red = "",
    green = "",
    blue = "",
    orange = "",
    purple = "",
    yellow = "",
    pink = "",
    grey = "",
    light_blue = "",
    light_grey = "",
}

--- Set the color palette that will be used
-- @param name string
function palette(name) end


--------------------------------------------------------------------------------
-- Color Module
--------------------------------------------------------------------------------

--- @class color
color = {}

--- Create a hex color string from RGB values
-- Accepts either three numbers (r, g, b) or one table {r=..., g=..., b=...}
-- @param r number|table: red component or table with r,g,b keys (0-255)
-- @param g number (optional): green component (0-255)
-- @param b number (optional): blue component (0-255)
-- @return string hex color string (e.g. "#FFAABB")
function color.rgb(r, g, b) end

--- Convert a hex color string (e.g. "#FFAABB") to an RGB table
-- @param hex string
-- @return table {r=number, g=number, b=number}
function color.to_rgb(hex) end

--- Create a hex color string from HSL values
-- @param h number: hue in degrees (0-360)
-- @param s number: saturation in percent (0-100)
-- @param l number: lightness in percent (0-100)
-- @return string hex color string
function color.hsl(h, s, l) end

--- Convert a hex color string to an HSL table
-- @param hex string
-- @return table {h=number, s=number, l=number}
function color.to_hsl(hex) end

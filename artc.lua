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
-- @param props table: {x, y, size, color, motion, speed, radius}
function modify(id, props) end

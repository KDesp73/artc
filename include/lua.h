#ifndef ARTC_LUA_H
#define ARTC_LUA_H

#include "lua/lua.h"

/*----------------.
| Creation        |
|      methods    |
`---------------*/
int lua_create_object(lua_State* L);
int lua_create_circle(lua_State* L);
int lua_create_square(lua_State* L);
int lua_create_triangle(lua_State* L);
int lua_create_line(lua_State* L);

/*----------------.
| Setters         |
|                 |
`---------------*/
int lua_set_background(lua_State* L);
int lua_set_dimensions(lua_State* L);
int lua_set_seed(lua_State* L);
int lua_set_fps(lua_State* L);

/*----------------.
| Getters         |
|                 |
`---------------*/
int lua_get_rand(lua_State* L);
int lua_get_time(lua_State* L);

/*----------------.
| Colors          |
|                 |
`---------------*/
int lua_rgb_object_to_hex(lua_State* L);
void register_color_module(lua_State* L);

/*----------------.
| Modifiers       |
|                 |
`---------------*/
int lua_modify_object(lua_State* L);
int lua_clear_scene(lua_State* L);


static inline void setup_lua(lua_State* L)
{
    lua_register(L, "shape", lua_create_object);
    lua_register(L, "circle", lua_create_circle);
    lua_register(L, "square", lua_create_square);
    lua_register(L, "triangle", lua_create_triangle);
    lua_register(L, "line", lua_create_line);

    lua_register(L, "bg", lua_set_background);
    lua_register(L, "window", lua_set_dimensions);
    lua_register(L, "seed", lua_set_seed);
    lua_register(L, "fps", lua_set_fps);

    lua_register(L, "time", lua_get_time);
    lua_register(L, "rand", lua_get_rand);

    lua_register(L, "hex", lua_rgb_object_to_hex); // NOTE: Kept for backwards compatibility reasons
    register_color_module(L);

    lua_register(L, "clear", lua_clear_scene);
    lua_register(L, "modify", lua_modify_object);
}

#endif // ARTC_LUA_H

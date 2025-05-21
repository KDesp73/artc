#ifndef ARTC_LUA_H
#define ARTC_LUA_H

#include "lua/lua.h"

int lua_create_object(lua_State* L);
int lua_create_circle(lua_State* L);
int lua_create_square(lua_State* L);
int lua_create_triangle(lua_State* L);
int lua_set_background(lua_State* L);
int lua_set_dimensions(lua_State* L);
int lua_get_time(lua_State* L);
int lua_rand(lua_State* L);
int lua_color_to_hex(lua_State* L);
int lua_clear_scene(lua_State* L);
int lua_modify_object(lua_State* L);
int lua_set_seed(lua_State* L);
// TODO: fps()

static inline void setup_lua(lua_State* L)
{
    lua_register(L, "object", lua_create_object);
    lua_register(L, "circle", lua_create_circle);
    lua_register(L, "square", lua_create_square);
    lua_register(L, "triangle", lua_create_triangle);
    lua_register(L, "bg", lua_set_background);
    lua_register(L, "time", lua_get_time);
    lua_register(L, "rand", lua_rand);
    lua_register(L, "window", lua_set_dimensions);
    lua_register(L, "hex", lua_color_to_hex);
    lua_register(L, "clear", lua_clear_scene);
    lua_register(L, "modify", lua_modify_object);
    lua_register(L, "seed", lua_set_seed);
}

#endif // ARTC_LUA_H

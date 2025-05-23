#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua.h"
#include "scene.h"

extern Scene scene;


int lua_set_background(lua_State* L)
{
    const char* color_str = luaL_checkstring(L, 1);
    scene.options.background = ParseHexColor(color_str);
    return 0;
}

int lua_set_dimensions(lua_State* L)
{
    int argc = lua_gettop(L);
    if (argc < 2)
        return luaL_error(L, "Expected 2 arguments: width and height");

    int width = luaL_checkinteger(L, 1);
    int height = luaL_checkinteger(L, 2);

    scene.options.width = width;
    scene.options.height = height;

    return 0;
}

int lua_set_seed(lua_State* L)
{
    if (!lua_isnumber(L, 1)) {
        return luaL_error(L, "Expected a number as seed");
    }

    unsigned int seed = (unsigned int)lua_tointeger(L, 1);

    srand(seed);

    return 0;
}

int lua_set_fps(lua_State* L)
{
    if (!lua_isnumber(L, 1)) {
        return luaL_error(L, "Expected a number as fps");
    }

    unsigned int fps = (unsigned int)lua_tointeger(L, 1);

    view.fps = fps;

    return 0;
}


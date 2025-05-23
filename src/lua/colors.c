#include "lua/lauxlib.h"
#include "lua/lua.h"

static int clamp(int v)
{
    if (v < 0) return 0;
    if (v > 255) return 255;
    return v;
}

int lua_color_to_hex(lua_State* L)
{
    if (!lua_istable(L, 1)) {
        return luaL_error(L, "Expected a table as argument");
    }

    lua_getfield(L, 1, "r");
    int r = (int)luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "g");
    int g = (int)luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "b");
    int b = (int)luaL_checknumber(L, -1);
    lua_pop(L, 1);

    r = clamp(r);
    g = clamp(g);
    b = clamp(b);

    char hex[8];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X", r, g, b);

    lua_pushstring(L, hex);
    return 1;
}


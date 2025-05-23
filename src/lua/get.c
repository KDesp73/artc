#include "SDL2/SDL_timer.h"
#include "lua/lauxlib.h"
#include "lua/lua.h"

int lua_get_time(lua_State* L)
{
    lua_pushnumber(L, (double)SDL_GetTicks());
    return 1;
}

int lua_get_rand(lua_State* L)
{
    double min = luaL_optnumber(L, 1, 0);
    double max = luaL_optnumber(L, 2, 1);
    double r = min + ((double)rand() / RAND_MAX) * (max - min);
    lua_pushnumber(L, r);
    return 1;
}


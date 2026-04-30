#include "files.h"
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua.h"
#include <stdlib.h>

int lua_read_file(lua_State* L)
{
    const char* path = luaL_checkstring(L, 1);
    char* buf = load_file(path);
    if (!buf) {
        lua_pushnil(L);
        return 1;
    }
    lua_pushstring(L, buf);
    free(buf);
    return 1;
}

int lua_file_exists(lua_State* L)
{
    const char* path = luaL_checkstring(L, 1);
    lua_pushboolean(L, file_exists(path) ? 1 : 0);
    return 1;
}

int lua_is_file(lua_State* L)
{
    const char* path = luaL_checkstring(L, 1);
    lua_pushboolean(L, is_file(path) ? 1 : 0);
    return 1;
}

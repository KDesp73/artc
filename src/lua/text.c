#include "lua.h"

void register_fonts_table(lua_State* L)
{
    lua_newtable(L);

    lua_pushstring(L, "artc:cozette");
    lua_setfield(L, -2, "cozette");

    lua_pushstring(L, "artc:funky");
    lua_setfield(L, -2, "funky");

    lua_setglobal(L, "font");
}


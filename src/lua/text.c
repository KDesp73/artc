#include "lua.h"

void register_fonts_table(lua_State* L)
{
    lua_newtable(L);

    lua_pushstring(L, "artc:cozette");
    lua_setfield(L, -2, "cozette");

    lua_pushstring(L, "artc:funky");
    lua_setfield(L, -2, "funky");

    lua_pushstring(L, "artc:aovel");
    lua_setfield(L, -2, "aovel");

    lua_pushstring(L, "artc:xfiles");
    lua_setfield(L, -2, "xfiles");

    lua_pushstring(L, "artc:xfiles-1");
    lua_setfield(L, -2, "xfiles_1");

    lua_pushstring(L, "artc:movistar");
    lua_setfield(L, -2, "movistar");

    lua_pushstring(L, "artc:fox");
    lua_setfield(L, -2, "fox");

    lua_setglobal(L, "font");
}


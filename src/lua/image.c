#include "lua.h"

void register_filters_table(lua_State* L)
{
    lua_newtable(L);

    lua_pushstring(L, "negative");
    lua_setfield(L, -2, "negative");

    lua_pushstring(L, "greyscale");
    lua_setfield(L, -2, "greyscale");

    lua_pushstring(L, "grayscale");
    lua_setfield(L, -2, "grayscale");

    lua_pushstring(L, "sepia");
    lua_setfield(L, -2, "sepia");

    lua_pushstring(L, "threshold");
    lua_setfield(L, -2, "threshold");

    lua_pushstring(L, "invert-red");
    lua_setfield(L, -2, "invert_red");

    lua_pushstring(L, "invert-green");
    lua_setfield(L, -2, "invert_green");

    lua_pushstring(L, "invert-blue");
    lua_setfield(L, -2, "invert_blue");

    lua_pushstring(L, "channel-swap");
    lua_setfield(L, -2, "channel_swap");

    lua_setglobal(L, "filter");
}

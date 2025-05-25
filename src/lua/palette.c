#include "palette.h"
#include "lua.h"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include <ctype.h>
#include <string.h>

static void lowercase(char* str)
{
    for (size_t i = 0; i < strlen(str); i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

static void register_palette_table(lua_State* L, Palette palette)
{
    lua_newtable(L);

    if (palette.red) {
        lua_pushstring(L, palette.red);
        lua_setfield(L, -2, "red");
    }
    if (palette.green) {
        lua_pushstring(L, palette.green);
        lua_setfield(L, -2, "green");
    }
    if (palette.blue) {
        lua_pushstring(L, palette.blue);
        lua_setfield(L, -2, "blue");
    }
    if (palette.yellow) {
        lua_pushstring(L, palette.yellow);
        lua_setfield(L, -2, "yellow");
    }
    if (palette.orange) {
        lua_pushstring(L, palette.orange);
        lua_setfield(L, -2, "orange");
    }
    if (palette.purple) {
        lua_pushstring(L, palette.purple);
        lua_setfield(L, -2, "purple");
    }
    if (palette.pink) {
        lua_pushstring(L, palette.pink);
        lua_setfield(L, -2, "pink");
    }
    if (palette.grey) {
        lua_pushstring(L, palette.grey);
        lua_setfield(L, -2, "grey");
    }
    if (palette.white) {
        lua_pushstring(L, palette.white);
        lua_setfield(L, -2, "white");
    }
    if (palette.black) {
        lua_pushstring(L, palette.black);
        lua_setfield(L, -2, "black");
    }
    if (palette.light_blue) {
        lua_pushstring(L, palette.light_blue);
        lua_setfield(L, -2, "light_blue");
    }
    if (palette.light_grey) {
        lua_pushstring(L, palette.light_grey);
        lua_setfield(L, -2, "light_grey");
    }


    lua_setglobal(L, "palette");
}

static void register_palette(lua_State* L, const char* name)
{
    char* palette = (char*) name;
    lowercase(palette);

    if(!strcmp("catppuccin", palette)) {
        register_palette_table(L, PaletteCatppuccinMacchiato());
    } else {
        luaL_error(L, "Unknown palette '%s'", name);
    }
}

int lua_set_palette(lua_State* L)
{
    const char* palette = luaL_checkstring(L, 1);
    register_palette(L, palette);

    return 0;
}


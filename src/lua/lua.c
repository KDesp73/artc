#include "lua/lua.h"
#include "io/logging.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
#include "lua.h"
#include "scene.h"
#include "view.h"

extern Scene scene;

void timeout_hook(lua_State *L, lua_Debug *ar)
{
    static int count = 0;
    if (++count > 100000) {
        luaL_error(L, "Script timed out");
    }
}

Scene SceneLoadLua(const char* filename, bool sandbox)
{
    memset(&scene, 0, sizeof(Scene));
    scene.options.width = 640;
    scene.options.height = 480;
    scene.options.background = (SDL_Color){0, 0, 0, 255};

    lua_State* L = luaL_newstate();
    if(sandbox){
        int libs_loaded = 0;
        luaL_requiref(L, "_G", luaopen_base, 1); libs_loaded++;
        luaL_requiref(L, "math", luaopen_math, 1); libs_loaded++;
        luaL_requiref(L, "table", luaopen_table, 1); libs_loaded++;
        luaL_requiref(L, "string", luaopen_string, 1); libs_loaded++;
        lua_pop(L, libs_loaded);

        lua_pushnil(L); lua_setglobal(L, "dofile");
        lua_pushnil(L); lua_setglobal(L, "loadfile");
        lua_pushnil(L); lua_setglobal(L, "require");
        lua_pushnil(L); lua_setglobal(L, "os");
        lua_pushnil(L); lua_setglobal(L, "io");
        lua_pushnil(L); lua_setglobal(L, "package");
        lua_pushnil(L); lua_setglobal(L, "debug");

        lua_sethook(L, timeout_hook, LUA_MASKCOUNT, 100000);
    } else {
        luaL_openlibs(L);
    }

    setup_lua(L);

    if (luaL_loadfile(L, filename) != LUA_OK) {
        ERRO("Lua load error: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return scene;
    }

    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        ERRO("Lua runtime error: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return scene;
    }

    lua_getglobal(L, "setup");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            ERRO("Lua setup() error: %s", lua_tostring(L, -1));
            lua_pop(L, 1);
            return scene;
        }
    } else {
        ERRO("Lua error: no 'setup' function defined");
        lua_pop(L, 1);
        return scene;
    }

    view.L = L;
    scene.loaded = true;
    return scene;
}


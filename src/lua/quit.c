#include "lua/lua.h"
#include "runtime.h"

int lua_quit(lua_State* L)
{
    (void)L;
    artc_request_quit();
    return 0;
}

#include "lua/lua.h"
#include "io/logging.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
#include "lua.h"
#include <stdlib.h>
#include <time.h>
#include "scene.h"
#include "view.h"

static Scene scene;

static int lua_create_object(lua_State* L, ObjectType type)
{
    if (!lua_istable(L, 1))
        return luaL_error(L, "Expected a table");

    if (scene.count >= MAX_OBJECTS)
        return luaL_error(L, "Maximum number of objects reached");

    ArtObject* obj = &scene.objects[scene.count++];
    obj->type = type;

    obj->id = scene.next_id++;

    // x (default 0)
    lua_getfield(L, 1, "x");
    obj->x = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    // y (default 0)
    lua_getfield(L, 1, "y");
    obj->y = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    // size (default 10)
    lua_getfield(L, 1, "size");
    obj->size = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 10.0f;
    lua_pop(L, 1);

    // color (default white)
    lua_getfield(L, 1, "color");
    if (lua_isstring(L, -1)) {
        obj->color = parse_color(lua_tostring(L, -1));
    } else {
        obj->color = parse_color("white");  // or your default color string
    }
    lua_pop(L, 1);

    // motion (default MOTION_STATIC)
    lua_getfield(L, 1, "motion");
    if (lua_isstring(L, -1)) {
        obj->motion = parse_motion(lua_tostring(L, -1));
    } else {
        obj->motion = MOTION_STATIC;
    }
    lua_pop(L, 1);

    // speed (default 0)
    lua_getfield(L, 1, "speed");
    obj->speed = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    // radius (default 0)
    lua_getfield(L, 1, "radius");
    obj->radius = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    obj->cx = obj->x;
    obj->cy = obj->y;

    lua_pushinteger(L, obj->id);
    return 1;
}

int lua_create_circle(lua_State* L) 
{
    return lua_create_object(L, OBJECT_CIRCLE);
}

int lua_create_square(lua_State* L)
{
    return lua_create_object(L, OBJECT_SQUARE);
}

int lua_set_background(lua_State* L)
{
    const char* color_str = luaL_checkstring(L, 1);
    scene.options.background = parse_color(color_str);
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

int lua_get_time(lua_State* L)
{
    lua_pushnumber(L, (double)SDL_GetTicks());
    return 1;
}

int lua_rand(lua_State* L)
{
    double min = luaL_optnumber(L, 1, 0);
    double max = luaL_optnumber(L, 2, 1);
    double r = min + ((double)rand() / RAND_MAX) * (max - min);
    lua_pushnumber(L, r);
    return 1;
}

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

int lua_clear_scene(lua_State* L)
{
    scene.count = 0;
    return 0;
}

static void push_color_table(lua_State* L, SDL_Color c)
{
    lua_newtable(L);
    lua_pushinteger(L, c.r);
    lua_setfield(L, -2, "r");
    lua_pushinteger(L, c.g);
    lua_setfield(L, -2, "g");
    lua_pushinteger(L, c.b);
    lua_setfield(L, -2, "b");
    lua_pushinteger(L, c.a);
    lua_setfield(L, -2, "a");
}

// Helper: read color table from Lua at index idx, set SDL_Color*
static int read_color_table(lua_State* L, int idx, SDL_Color* c)
{
    if (!lua_istable(L, idx)) return 0;
    lua_getfield(L, idx, "r");
    c->r = (Uint8)luaL_optinteger(L, -1, c->r);
    lua_pop(L, 1);
    lua_getfield(L, idx, "g");
    c->g = (Uint8)luaL_optinteger(L, -1, c->g);
    lua_pop(L, 1);
    lua_getfield(L, idx, "b");
    c->b = (Uint8)luaL_optinteger(L, -1, c->b);
    lua_pop(L, 1);
    lua_getfield(L, idx, "a");
    c->a = (Uint8)luaL_optinteger(L, -1, 255);
    lua_pop(L, 1);
    return 1;
}

// Metamethod __index for ArtObject userdata
static int artobject_index(lua_State* L)
{
    ArtObject* obj = (ArtObject*)luaL_checkudata(L, 1, "ArtObjectMT");
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0)
        lua_pushnumber(L, obj->x);
    else if (strcmp(key, "y") == 0)
        lua_pushnumber(L, obj->y);
    else if (strcmp(key, "cx") == 0)
        lua_pushnumber(L, obj->cx);
    else if (strcmp(key, "cy") == 0)
        lua_pushnumber(L, obj->cy);
    else if (strcmp(key, "size") == 0)
        lua_pushnumber(L, obj->size);
    else if (strcmp(key, "color") == 0)
        push_color_table(L, obj->color);
    else if (strcmp(key, "motion") == 0)
        lua_pushstring(L, motion_to_string(obj->motion));
    else if (strcmp(key, "type") == 0)
        lua_pushstring(L, object_type_to_string(obj->type));
    else if (strcmp(key, "speed") == 0)
        lua_pushnumber(L, obj->speed);
    else if (strcmp(key, "radius") == 0)
        lua_pushnumber(L, obj->radius);
    else
        lua_pushnil(L);

    return 1;
}

// Metamethod __newindex for ArtObject userdata
static int artobject_newindex(lua_State* L) {
    ArtObject* obj = (ArtObject*)luaL_checkudata(L, 1, "ArtObjectMT");
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0)
        obj->x = (float)luaL_checknumber(L, 3);
    else if (strcmp(key, "y") == 0)
        obj->y = (float)luaL_checknumber(L, 3);
    else if (strcmp(key, "cx") == 0)
        obj->cx = (float)luaL_checknumber(L, 3);
    else if (strcmp(key, "cy") == 0)
        obj->cy = (float)luaL_checknumber(L, 3);
    else if (strcmp(key, "size") == 0)
        obj->size = (float)luaL_checknumber(L, 3);
    else if (strcmp(key, "color") == 0) {
        if (!read_color_table(L, 3, &obj->color)) {
            return luaL_error(L, "color must be a table with r,g,b[,a]");
        }
    }
    else if (strcmp(key, "motion") == 0) {
        const char* mstr = luaL_checkstring(L, 3);
        obj->motion = parse_motion(mstr); // your motion parser
    }
    else if (strcmp(key, "speed") == 0)
        obj->speed = (float)luaL_checknumber(L, 3);
    else if (strcmp(key, "radius") == 0)
        obj->radius = (float)luaL_checknumber(L, 3);
    else {
        return luaL_error(L, "Attempt to set invalid property '%s'", key);
    }

    return 0;
}

// Lua function to get object userdata by index
int lua_get_object(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
    if (index < 1 || index > scene.count) {
        lua_pushnil(L);
        return 1;
    }

    ArtObject* obj = &scene.objects[index - 1];

    // Create userdata wrapping the ArtObject pointer
    ArtObject** udata = (ArtObject**)lua_newuserdata(L, sizeof(ArtObject*));
    *udata = obj;

    // Set metatable
    luaL_getmetatable(L, "ArtObjectMT");
    lua_setmetatable(L, -2);

    return 1; // userdata on stack
}

// Call this once to register metatable & function
void register_artobject(lua_State* L)
{
    // Create metatable
    luaL_newmetatable(L, "ArtObjectMT");

    // Set __index
    lua_pushcfunction(L, artobject_index);
    lua_setfield(L, -2, "__index");

    // Set __newindex
    lua_pushcfunction(L, artobject_newindex);
    lua_setfield(L, -2, "__newindex");

    lua_pop(L, 1); // pop metatable

    // Register global function
    lua_register(L, "get_object", lua_get_object);
}

static ArtObject* find_object_by_id(int id) {
    for (int i = 0; i < scene.count; i++) {
        if (scene.objects[i].id == id)
            return &scene.objects[i];
    }
    return NULL;
}

int lua_modify_object(lua_State* L)
{
    // Args: (int) id, (table) props
    int id = luaL_checkinteger(L, 1);
    if (id < 0 || id >= scene.count) {
        return luaL_error(L, "Invalid object id");
    }

    if (!lua_istable(L, 2)) {
        return luaL_error(L, "Expected table as second argument");
    }

    ArtObject* obj = find_object_by_id(id);

    lua_getfield(L, 2, "x");
    if (lua_isnumber(L, -1)) obj->x = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "y");
    if (lua_isnumber(L, -1)) obj->y = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "size");
    if (lua_isnumber(L, -1)) obj->size = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "speed");
    if (lua_isnumber(L, -1)) obj->speed = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "radius");
    if (lua_isnumber(L, -1)) obj->radius = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "motion");
    if (lua_isstring(L, -1)) {
        const char* motion_str = lua_tostring(L, -1);
        obj->motion = parse_motion(motion_str);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "color");
    if (lua_isstring(L, -1)) {
        const char* color_str = lua_tostring(L, -1);
        obj->color = parse_color(color_str);
    }
    lua_pop(L, 1);

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

Scene SceneLoadLua(const char* filename)
{
    memset(&scene, 0, sizeof(Scene));
    scene.options.width = 640;
    scene.options.height = 480;
    scene.options.background = (SDL_Color){0, 0, 0, 255};

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

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
        }
    } else {
        ERRO("Lua error: no 'setup' function defined");
        lua_pop(L, 1);
    }

    view.L = L;
    scene.loaded = true;
    return scene;
}


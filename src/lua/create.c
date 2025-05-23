#include "lua/lua.h"
#include "entities.h"
#include "lua/lauxlib.h"
#include "lua.h"
#include "scene.h"

extern Scene scene;

int lua_create_object(lua_State* L)
{
    if (!lua_istable(L, 1))
        return luaL_error(L, "Expected a table");

    if (scene.count >= MAX_ENTITIES)
        return luaL_error(L, "Maximum number of entities reached");

    ArtEntity* entity = &scene.entities[scene.count++];
    ArtShape* obj = &entity->shape;

    entity->id = scene.next_id++;

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
        obj->color = ParseHexColor(lua_tostring(L, -1));
    } else {
        obj->color = ParseHexColor("#ffffff");
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "type");
    if (lua_isstring(L, -1)) {
        obj->type = ParseShapeType(lua_tostring(L, -1));
    } else {
        obj->type = SHAPE_SQUARE;
    }
    lua_pop(L, 1);

    // motion (default MOTION_STATIC)
    lua_getfield(L, 1, "motion");
    if (lua_isstring(L, -1)) {
        obj->motion = ParseMotion(lua_tostring(L, -1));
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

    lua_pushinteger(L, entity->id);
    return 1;

}

static int create_object(lua_State* L, ShapeType type)
{
    if (!lua_istable(L, 1))
        return luaL_error(L, "Expected a table");

    if (scene.count >= MAX_ENTITIES)
        return luaL_error(L, "Maximum number of entities reached");

    ArtEntity* entity = &scene.entities[scene.count++];
    ArtShape* obj = &entity->shape;
    obj->type = type;

    entity->id = scene.next_id++;
    entity->kind = ENTITY_OBJECT;

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
        obj->color = ParseHexColor(lua_tostring(L, -1));
    } else {
        obj->color = ParseHexColor("#ffffff");
    }
    lua_pop(L, 1);

    // motion (default MOTION_STATIC)
    lua_getfield(L, 1, "motion");
    if (lua_isstring(L, -1)) {
        obj->motion = ParseMotion(lua_tostring(L, -1));
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

    lua_pushinteger(L, entity->id);
    return 1;
}

int lua_create_circle(lua_State* L) 
{
    return create_object(L, SHAPE_CIRCLE);
}

int lua_create_square(lua_State* L)
{
    return create_object(L, SHAPE_SQUARE);
}

int lua_create_triangle(lua_State* L)
{
    return create_object(L, SHAPE_TRIANGLE);
}

int lua_create_line(lua_State* L)
{
    if (!lua_istable(L, 1))
        return luaL_error(L, "Expected a table");

    if (scene.count >= MAX_ENTITIES)
        return luaL_error(L, "Maximum number of entities reached");

    ArtEntity* entity = &scene.entities[scene.count++];
    ArtLine* line = &entity->line;
    entity->kind = ENTITY_LINE;
    entity->id = scene.next_id++;

    // -- @param opts table: {x1, y1, x2, y2, color}

    // x1 (default 0)
    lua_getfield(L, 1, "x1");
    line->x1 = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    // y1 (default 0)
    lua_getfield(L, 1, "y1");
    line->y1 = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    // x2 (default 0)
    lua_getfield(L, 1, "x2");
    line->x2 = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    // y2 (default 0)
    lua_getfield(L, 1, "y2");
    line->y2 = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    // color (default white)
    lua_getfield(L, 1, "color");
    if (lua_isstring(L, -1)) {
        line->color = ParseHexColor(lua_tostring(L, -1));
    } else {
        line->color = ParseHexColor("#ffffff");
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "thickness");
    line->thickness = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    lua_pushinteger(L, entity->id);
    return 1;
}

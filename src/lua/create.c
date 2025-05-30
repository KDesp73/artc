#include "SDL2/SDL_image.h"
#include "lua/lua.h"
#include "entities.h"
#include "lua/lauxlib.h"
#include "lua.h"
#include "scene.h"
#include <stdio.h>

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

int lua_create_image(lua_State* L)
{
    if (!lua_istable(L, 1))
        return luaL_error(L, "Expected a table");

    if (scene.count >= MAX_ENTITIES)
        return luaL_error(L, "Maximum number of entities reached");

    ArtEntity* entity = &scene.entities[scene.count++];
    ArtImage* image = &entity->image;
    entity->kind = ENTITY_IMAGE;
    entity->id = scene.next_id++;

    // Free old strings if any
    if (image->filter) {
        free(image->filter);
        image->filter = NULL;
    }
    if (image->src) {
        free(image->src);
        image->src = NULL;
    }

    // x (default 0)
    lua_getfield(L, 1, "x");
    image->x = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    // y (default 0)
    lua_getfield(L, 1, "y");
    image->y = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    // w (default 0)
    lua_getfield(L, 1, "w");
    image->w = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    // h (default 0)
    lua_getfield(L, 1, "h");
    image->h = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    // filter (default NULL)
    lua_getfield(L, 1, "filter");
    if (lua_isstring(L, -1)) {
        const char* s = lua_tostring(L, -1);
        image->filter = strdup(s);
    }
    lua_pop(L, 1);

    // motion (default MOTION_STATIC)
    lua_getfield(L, 1, "motion");
    if (lua_isstring(L, -1)) {
        image->motion = ParseMotion(lua_tostring(L, -1));
    } else {
        image->motion = MOTION_STATIC;
    }
    lua_pop(L, 1);

    // speed (default 0)
    lua_getfield(L, 1, "speed");
    image->speed = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    // radius (default 0)
    lua_getfield(L, 1, "radius");
    image->radius = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    // src (image path, required)
    lua_getfield(L, 1, "src");
    if (lua_isstring(L, -1)) {
        const char* s = lua_tostring(L, -1);
        image->src = strdup(s);
    } else {
        lua_pop(L, 1);
        return luaL_error(L, "Image 'src' is required and must be a string");
    }
    lua_pop(L, 1);

    lua_pushinteger(L, entity->id);
    return 1;
}

int lua_create_text(lua_State* L)
{
    if (!lua_istable(L, 1))
        return luaL_error(L, "Expected a table");

    if (scene.count >= MAX_ENTITIES)
        return luaL_error(L, "Maximum number of entities reached");

    ArtEntity* entity = &scene.entities[scene.count++];
    ArtText* text = &entity->text;
    entity->kind = ENTITY_TEXT;
    entity->id = scene.next_id++;

    // Free old strings if any
    if (text->font) {
        free(text->font);
        text->font = NULL;
    }
    if (text->content) {
        free(text->content);
        text->content = NULL;
    }

    // x (default 0)
    lua_getfield(L, 1, "x");
    text->x = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    // y (default 0)
    lua_getfield(L, 1, "y");
    text->y = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0f;
    lua_pop(L, 1);

    // size (default 12)
    lua_getfield(L, 1, "size");
    text->font_size = lua_isnumber(L, -1) ? (size_t)lua_tointeger(L, -1) : 12;
    lua_pop(L, 1);

    // font (default NULL)
    lua_getfield(L, 1, "font");
    if (lua_isstring(L, -1)) {
        const char* s = lua_tostring(L, -1);
        text->font = strdup(s);
    } else {
        text->font = NULL;
    }
    lua_pop(L, 1);

    // fg (default "#ffffff")
    lua_getfield(L, 1, "fg");
    const char* fg_str = lua_isstring(L, -1) ? lua_tostring(L, -1) : "#ffffff";
    text->fg = ParseHexColor(fg_str);
    lua_pop(L, 1);

    // bg (default "#000000")
    lua_getfield(L, 1, "bg");
    const char* bg_str = lua_isstring(L, -1) ? lua_tostring(L, -1) : "#000000";
    text->bg = ParseHexColor(bg_str);
    lua_pop(L, 1);

    // content (required)
    lua_getfield(L, 1, "content");
    if (lua_isstring(L, -1)) {
        const char* s = lua_tostring(L, -1);
        text->content = strdup(s);
    } else {
        lua_pop(L, 1);
        return luaL_error(L, "Text 'content' is required and must be a string");
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "blend");
    text->blend = lua_toboolean(L, -1);
    lua_pop(L, 1);

    lua_pushinteger(L, entity->id);
    return 1;
}

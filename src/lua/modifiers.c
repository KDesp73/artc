#include "lua/lua.h"
#include "entities.h"
#include "lua/lauxlib.h"
#include "lua.h"
#include <time.h>
#include "scene.h"

extern Scene scene;

static ArtEntity* find_entity_by_id(int id) {
    for (int i = 0; i < scene.count; i++) {
        if (scene.entities[i].id == id)
            return &scene.entities[i];
    }
    return NULL;
}

int lua_modify_entity(lua_State* L)
{
    // Args: (int) id, (table) props
    int id = luaL_checkinteger(L, 1);

    if (!lua_istable(L, 2)) {
        return luaL_error(L, "Expected table as second argument");
    }

    ArtEntity* entity = find_entity_by_id(id);
    if (entity == NULL) {
        return luaL_error(L, "Could not find object with id: %d", id);
    }

    if (entity->kind == ENTITY_OBJECT) {
        ArtShape* obj = &entity->shape;

        lua_getfield(L, 2, "x");
        if (lua_isnumber(L, -1)) obj->x = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "y");
        if (lua_isnumber(L, -1)) obj->y = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "size");
        if (lua_isnumber(L, -1)) obj->size = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "speed");
        if (lua_isnumber(L, -1)) obj->speed = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "radius");
        if (lua_isnumber(L, -1)) obj->radius = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "motion");
        if (lua_isstring(L, -1)) {
            const char* motion_str = lua_tostring(L, -1);
            obj->motion = ParseMotion(motion_str);
        }
        lua_pop(L, 1);

        lua_getfield(L, 2, "color");
        if (lua_isstring(L, -1)) {
            const char* color_str = lua_tostring(L, -1);
            obj->color = ParseHexColor(color_str);
        }
        lua_pop(L, 1);

        lua_getfield(L, 2, "type");
        if (lua_isstring(L, -1)) {
            const char* type_str = lua_tostring(L, -1);
            obj->type = ParseShapeType(type_str);
        }
        lua_pop(L, 1);

    } else if (entity->kind == ENTITY_LINE) {
        ArtLine* line = &entity->line;

        lua_getfield(L, 2, "x1");
        if (lua_isnumber(L, -1)) line->x1 = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "y1");
        if (lua_isnumber(L, -1)) line->y1 = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "x2");
        if (lua_isnumber(L, -1)) line->x2 = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "y2");
        if (lua_isnumber(L, -1)) line->y2 = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "color");
        if (lua_isstring(L, -1)) {
            const char* color_str = lua_tostring(L, -1);
            line->color = ParseHexColor(color_str);
        }
        lua_pop(L, 1);

        lua_getfield(L, 2, "thickness");
        if (lua_isnumber(L, -1)) line->thickness = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);
    } else if(entity->kind == ENTITY_IMAGE){
        ArtImage* img = &entity->image;

        lua_getfield(L, 2, "x");
        if (lua_isnumber(L, -1)) img->x = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "y");
        if (lua_isnumber(L, -1)) img->y = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "w");
        if (lua_isnumber(L, -1)) img->w = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "h");
        if (lua_isnumber(L, -1)) img->h = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "filter");
        if (lua_isstring(L, -1)) {
            const char* filter_str = lua_tostring(L, -1);
            if (img->filter) free(img->filter);
            img->filter = strdup(filter_str);
        }
        lua_pop(L, 1);

        lua_getfield(L, 2, "motion");
        if (lua_isstring(L, -1)) {
            img->motion = ParseMotion(lua_tostring(L, -1));
        }
        lua_pop(L, 1);

        lua_getfield(L, 2, "speed");
        if (lua_isnumber(L, -1)) img->speed = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "radius");
        if (lua_isnumber(L, -1)) img->radius = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "src");
        if (lua_isstring(L, -1)) {
            const char* src_str = lua_tostring(L, -1);
            if (img->src) free(img->src);
            img->src = strdup(src_str);
            // You might want to reload the texture here after changing the src
        }
        lua_pop(L, 1);
    } else {
        return luaL_error(L, "Unknown entity type");
    }

    return 0;
}

int lua_clear_scene(lua_State* L)
{
    scene.count = 0;
    return 0;
}

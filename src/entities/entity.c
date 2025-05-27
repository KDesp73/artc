#include "entities.h"

void EntityPaint(ArtEntity* e, View* view)
{
    switch(e->kind){
        case ENTITY_LINE:
            LinePaint(&e->line, view);
            break;
        case ENTITY_OBJECT:
            ShapePaint(&e->shape, view);
            break;
        case ENTITY_IMAGE:
            ImagePaint(&e->image, view);
            break;
        default:
            break;
    }
}

void EntityUpdate(ArtEntity* e, float time)
{
    if(e->kind == ENTITY_LINE) return;
    
    ShapeUpdate(&e->shape, time);
}

SDL_Color ParseHexColor(const char* hex)
{
    SDL_Color color = {255, 255, 255, 255};

    if (!hex || hex[0] != '#' || strlen(hex) != 7) {
        return color;
    }

    char r[3] = { hex[1], hex[2], '\0' };
    char g[3] = { hex[3], hex[4], '\0' };
    char b[3] = { hex[5], hex[6], '\0' };

    color.r = (Uint8)strtol(r, NULL, 16);
    color.g = (Uint8)strtol(g, NULL, 16);
    color.b = (Uint8)strtol(b, NULL, 16);

    return color;
}


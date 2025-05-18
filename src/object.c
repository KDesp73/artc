#include "art-object.h"

void ObjectUpdate(ArtObject* o, float time)
{
    switch (o->motion) {
        case MOTION_SPIN:
            o->x = o->cx + cos(time * o->speed) * o->radius;
            o->y = o->cy + sin(time * o->speed) * o->radius;
            break;
        case MOTION_DRIFT:
            o->x += cos(time) * o->speed;
            o->y += sin(time) * o->speed;
            break;
        case MOTION_PULSE:
            o->size = fabs(sin(time * o->speed)) * o->radius;
            break;
        default: break;
    }
}

void ObjectPaint(ArtObject* o, View* view)
{
    SDL_SetRenderDrawColor(view->renderer, o->color.r, o->color.g, o->color.b, 255);

    if (o->type == OBJECT_SQUARE) {
        SDL_Rect r = { (int)(o->x - o->size / 2), (int)(o->y - o->size / 2), (int)o->size, (int)o->size };
        SDL_RenderFillRect(view->renderer, &r);
    }
    else if (o->type == OBJECT_CIRCLE) {
        int radius = (int)(o->size / 2);
        int cx = (int)o->x;
        int cy = (int)o->y;

        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx*dx + dy*dy <= radius*radius) {
                    SDL_RenderDrawPoint(view->renderer, cx + dx, cy + dy);
                }
            }
        }
    }
}

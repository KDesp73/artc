#include "art-object.h"
#include <math.h>

void ObjectUpdate(ArtObject* o, float time)
{
    switch (o->motion) {
        case MOTION_STATIC: return;
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

        default:
            break;
    }
}

void ObjectPaint(ArtObject* o, View* view)
{
    Uint32 color = SDL_MapRGBA(view->surface->format, o->color.r, o->color.g, o->color.b, 255);

    if (o->type == OBJECT_SQUARE) {
        SDL_Rect r = { (int)o->x, (int)o->y, (int)o->size, (int)o->size };
        SDL_FillRect(view->surface, &r, color);
    } else if (o->type == OBJECT_CIRCLE) {
        int cx = (int)o->x;
        int cy = (int)o->y;
        int r = (int)o->size;
        Uint32* pixels = (Uint32*)view->surface->pixels;
        int pitch = view->surface->pitch / 4;

        for (int y = -r; y <= r; y++) {
            for (int x = -r; x <= r; x++) {
                if (x*x + y*y <= r*r) {
                    int px = cx + x;
                    int py = cy + y;
                    if (px >= 0 && px < view->width && py >= 0 && py < view->height) {
                        pixels[py * pitch + px] = color;
                    }
                }
            }
        }
    }
}

char* motion_to_string(MotionType motion)
{
    switch (motion) {
    case MOTION_STATIC: return "static";
    case MOTION_SPIN: return "spin";
    case MOTION_DRIFT: return "drift";
    case MOTION_PULSE: return "pulse";
    case MOTION_BOUNCE: return "bounce";
    case MOTION_WAVE: return "wave";
    case MOTION_ZIGZAG: return "zigzag";
    case MOTION_SWIRL: return "swirl";
    case MOTION_NOISE: return "noise";
    default: return "static";
    }
}

char* object_type_to_string(ObjectType obj)
{
    switch (obj) {
        case OBJECT_SQUARE: return "square";
        case OBJECT_CIRCLE: return "circle";
        case OBJECT_NONE:
        default:
            return "none";
    }
}

void ObjectPrint(ArtObject* o)
{
    if (!o) {
        printf("Object is NULL\n");
        return;
    }

    printf("ArtObject id=%d type=%d\n", o->id, o->type);
    printf("  Position: (%.2f, %.2f)\n", o->x, o->y);
    printf("  Size: %.2f\n", o->size);
    printf("  Color: R=%d G=%d B=%d\n", o->color.r, o->color.g, o->color.b);
    printf("  Motion: %d\n", o->motion);
    printf("  Speed: %.2f\n", o->speed);
    printf("  Radius: %.2f\n", o->radius);
    printf("  Center: (%.2f, %.2f)\n", o->cx, o->cy);
}

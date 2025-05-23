#include "art-object.h"
#include <math.h>

#include <math.h>
#include <stdlib.h>

void ObjectUpdate(ArtObject* o, float time)
{
    switch (o->motion) {
        case MOTION_STATIC:
            return;

        case MOTION_SPIN:
            o->x = o->cx + cosf(time * o->speed) * o->radius;
            o->y = o->cy + sinf(time * o->speed) * o->radius;
            break;

        case MOTION_DRIFT:
            o->x += cosf(time) * o->speed;
            o->y += sinf(time) * o->speed;
            break;

        case MOTION_PULSE:
            o->size = fabsf(sinf(time * o->speed)) * o->radius;
            break;

        case MOTION_BOUNCE:
            o->x = o->cx;
            o->y = o->cy + fabsf(sinf(time * o->speed)) * o->radius;
            break;

        case MOTION_WAVE:
            o->x = o->cx + sinf(time * o->speed) * o->radius;
            o->y = o->cy;
            break;

        case MOTION_ZIGZAG:
            o->x = o->cx + (fmodf(time * o->speed, 2.0f) < 1.0f ? 1 : -1) * o->radius;
            o->y = o->cy + sinf(time * o->speed) * o->radius * 0.5f;
            break;

        case MOTION_SWIRL:
            {
                float r = o->radius + sinf(time * o->speed) * o->radius * 0.5f;
                o->x = o->cx + cosf(time * o->speed) * r;
                o->y = o->cy + sinf(time * o->speed) * r;
            }
            break;

        case MOTION_NOISE:
            o->x = o->cx + ((rand() % 200) / 100.0f - 1.0f) * o->radius * 0.2f;
            o->y = o->cy + ((rand() % 200) / 100.0f - 1.0f) * o->radius * 0.2f;
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
    } else if (o->type == OBJECT_TRIANGLE) {
        int cx = (int)o->x;
        int cy = (int)o->y;
        int size = (int)o->size;
        Uint32* pixels = (Uint32*)view->surface->pixels;
        int pitch = view->surface->pitch / 4;

        for (int y = 0; y < size; y++) {
            int start_x = cx - y / 2;
            int end_x = cx + y / 2;
            int py = cy + y;
            for (int px = start_x; px <= end_x; px++) {
                if (px >= 0 && px < view->width && py >= 0 && py < view->height) {
                    pixels[py * pitch + px] = color;
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
        case OBJECT_TRIANGLE: return "triangle";
        case OBJECT_NONE:
        default:
            return "none";
    }
}

ObjectType parse_object_type(const char* str)
{
    if(!strcmp(str, object_type_to_string(OBJECT_CIRCLE))) return OBJECT_CIRCLE;
    if(!strcmp(str, object_type_to_string(OBJECT_TRIANGLE))) return OBJECT_TRIANGLE;
    if(!strcmp(str, object_type_to_string(OBJECT_SQUARE))) return OBJECT_SQUARE;
    return OBJECT_NONE;
}

void ObjectPrint(ArtEntity* e)
{
    ArtObject* o = &e->object;
    if (!o) {
        printf("Object is NULL\n");
        return;
    }

    printf("ArtObject id=%d type=%s\n", e->id, object_type_to_string(o->type));
    printf("  Position: (%.2f, %.2f)\n", o->x, o->y);
    printf("  Size: %.2f\n", o->size);
    printf("  Color: R=%d G=%d B=%d\n", o->color.r, o->color.g, o->color.b);
    printf("  Motion: %d\n", o->motion);
    printf("  Speed: %.2f\n", o->speed);
    printf("  Radius: %.2f\n", o->radius);
    printf("  Center: (%.2f, %.2f)\n", o->cx, o->cy);
}

void LinePaint(ArtLine* l, View* view)
{
    if (!l || !view || !view->surface) return;

    Uint32 color = SDL_MapRGBA(view->surface->format, l->color.r, l->color.g, l->color.b, l->color.a);

    Uint32* pixels = (Uint32*)view->surface->pixels;
    int pitch = view->surface->pitch / 4;

    float x1 = l->x1;
    float y1 = l->y1;
    float x2 = l->x2;
    float y2 = l->y2;
    float thickness = l->thickness;

    float dx = x2 - x1;
    float dy = y2 - y1;
    float length = sqrtf(dx * dx + dy * dy);
    if (length == 0) return;

    float half_thickness = thickness / 2.0f;

    int steps = (int)length;

    for (int i = 0; i <= steps; i++) {
        float t = (float)i / (float)steps;
        float cx = x1 + t * dx;
        float cy = y1 + t * dy;

        int radius = (int)(half_thickness);

        int start_x = (int)(cx - radius);
        int end_x = (int)(cx + radius);
        int start_y = (int)(cy - radius);
        int end_y = (int)(cy + radius);

        for (int py = start_y; py <= end_y; py++) {
            if (py < 0 || py >= view->height) continue;

            for (int px = start_x; px <= end_x; px++) {
                if (px < 0 || px >= view->width) continue;

                int dxp = px - (int)cx;
                int dyp = py - (int)cy;

                if (dxp*dxp + dyp*dyp <= radius*radius) {
                    pixels[py * pitch + px] = color;
                }
            }
        }
    }
}

void EntityPaint(ArtEntity* e, View* view)
{
    switch(e->kind){
        case ENTITY_LINE:
            LinePaint(&e->line, view);
            break;
        case ENTITY_OBJECT:
            ObjectPaint(&e->object, view);
            break;
        default:
            break;
    }
}
void EntityUpdate(ArtEntity* e, float time)
{
    if(e->kind == ENTITY_LINE) return;
    
    ObjectUpdate(&e->object, time);
}


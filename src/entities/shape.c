#include "entities.h"

void ShapeUpdate(ArtShape* o, float time)
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

void ShapePaint(ArtShape* o, View* view)
{
    Uint32 color = SDL_MapRGBA(view->surface->format, o->color.r, o->color.g, o->color.b, 255);

    if (o->type == SHAPE_SQUARE) {
        SDL_Rect r = { (int)o->x, (int)o->y, (int)o->size, (int)o->size };
        SDL_FillRect(view->surface, &r, color);
    } else if (o->type == SHAPE_CIRCLE) {
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
    } else if (o->type == SHAPE_TRIANGLE) {
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

char* Motion2Str(MotionType motion)
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

char* ShapeType2Str(ShapeType obj)
{
    switch (obj) {
        case SHAPE_SQUARE: return "square";
        case SHAPE_CIRCLE: return "circle";
        case SHAPE_TRIANGLE: return "triangle";
        case SHAPE_NONE:
        default:
            return "none";
    }
}

ShapeType ParseShapeType(const char* str)
{
    if(!strcmp(str, ShapeType2Str(SHAPE_CIRCLE))) return SHAPE_CIRCLE;
    if(!strcmp(str, ShapeType2Str(SHAPE_TRIANGLE))) return SHAPE_TRIANGLE;
    if(!strcmp(str, ShapeType2Str(SHAPE_SQUARE))) return SHAPE_SQUARE;
    return SHAPE_NONE;
}

void ShapePrint(ArtEntity* e)
{
    ArtShape* o = &e->shape;
    if (!o) {
        printf("Object is NULL\n");
        return;
    }

    printf("ArtObject id=%d type=%s\n", e->id, ShapeType2Str(o->type));
    printf("  Position: (%.2f, %.2f)\n", o->x, o->y);
    printf("  Size: %.2f\n", o->size);
    printf("  Color: R=%d G=%d B=%d\n", o->color.r, o->color.g, o->color.b);
    printf("  Motion: %d\n", o->motion);
    printf("  Speed: %.2f\n", o->speed);
    printf("  Radius: %.2f\n", o->radius);
    printf("  Center: (%.2f, %.2f)\n", o->cx, o->cy);
}

MotionType ParseMotion(const char* s)
{
    if (strcmp(s, "spin") == 0) return MOTION_SPIN;
    if (strcmp(s, "drift") == 0) return MOTION_DRIFT;
    if (strcmp(s, "pulse") == 0) return MOTION_PULSE;
    if (strcmp(s, "swirl") == 0) return MOTION_SWIRL;
    if (strcmp(s, "bounce") == 0) return MOTION_BOUNCE;
    if (strcmp(s, "noise") == 0) return MOTION_NOISE;
    if (strcmp(s, "zigzag") == 0) return MOTION_ZIGZAG;
    if (strcmp(s, "wave") == 0) return MOTION_WAVE;

    return MOTION_STATIC;
}


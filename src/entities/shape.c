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
    SDL_Renderer* renderer = view->renderer;

    Uint8 r = o->color.r;
    Uint8 g = o->color.g;
    Uint8 b = o->color.b;
    Uint8 a = 255;

    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    if (o->type == SHAPE_SQUARE) {
        SDL_Rect r = { (int)o->x, (int)o->y, (int)o->size, (int)o->size };
        SDL_RenderFillRect(renderer, &r);

    } else if (o->type == SHAPE_RECTANGLE) {
        SDL_Rect r = { (int)o->x, (int)o->y, (int)o->w, (int)o->h };
        SDL_RenderFillRect(renderer, &r);

    } else if (o->type == SHAPE_TRIANGLE) {
        float cx = o->x;
        float cy = o->y;
        float half_w = o->w / 2.0f;
        float h = o->h;

        SDL_Vertex vertices[3];

        vertices[0].position.x = cx;
        vertices[0].position.y = cy;
        vertices[0].color.r = r;
        vertices[0].color.g = g;
        vertices[0].color.b = b;
        vertices[0].color.a = a;
        vertices[0].tex_coord.x = 0;
        vertices[0].tex_coord.y = 0;

        vertices[1].position.x = cx - half_w;
        vertices[1].position.y = cy + h;
        vertices[1].color = vertices[0].color;
        vertices[1].tex_coord = vertices[0].tex_coord;

        vertices[2].position.x = cx + half_w;
        vertices[2].position.y = cy + h;
        vertices[2].color = vertices[0].color;
        vertices[2].tex_coord = vertices[0].tex_coord;

        SDL_RenderGeometry(renderer, NULL, vertices, 3, NULL, 0);

    } else if (o->type == SHAPE_CIRCLE) {
        // Approximate circle by polygon of many vertices

        const int num_segments = 40;
        SDL_Vertex vertices[num_segments];
        float cx = o->x;
        float cy = o->y;
        float radius = o->size;

        for (int i = 0; i < num_segments; i++) {
            float theta = (2.0f * M_PI * i) / num_segments;
            vertices[i].position.x = cx + radius * cosf(theta);
            vertices[i].position.y = cy + radius * sinf(theta);
            vertices[i].color.r = r;
            vertices[i].color.g = g;
            vertices[i].color.b = b;
            vertices[i].color.a = a;
            vertices[i].tex_coord.x = 0;
            vertices[i].tex_coord.y = 0;
        }

        // Draw triangle fan
        for (int i = 1; i < num_segments - 1; i++) {
            SDL_Vertex tri[3] = { vertices[0], vertices[i], vertices[i+1] };
            SDL_RenderGeometry(renderer, NULL, tri, 3, NULL, 0);
        }

    } else if (o->type == SHAPE_ELLIPSE) {
        // Approximate ellipse like circle but scale x and y separately

        const int num_segments = 40;
        SDL_Vertex vertices[num_segments];
        float cx = o->x;
        float cy = o->y;
        float a = o->w / 2.0f;
        float b = o->h / 2.0f;

        for (int i = 0; i < num_segments; i++) {
            float theta = (2.0f * M_PI * i) / num_segments;
            vertices[i].position.x = cx + a * cosf(theta);
            vertices[i].position.y = cy + b * sinf(theta);
            vertices[i].color.r = r;
            vertices[i].color.g = g;
            vertices[i].color.b = b;
            vertices[i].color.a = a;
            vertices[i].tex_coord.x = 0;
            vertices[i].tex_coord.y = 0;
        }

        for (int i = 1; i < num_segments - 1; i++) {
            SDL_Vertex tri[3] = { vertices[0], vertices[i], vertices[i + 1] };
            SDL_RenderGeometry(renderer, NULL, tri, 3, NULL, 0);
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


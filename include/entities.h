#ifndef ART_OBJECT_H
#define ART_OBJECT_H

#include "view.h"
#include <SDL2/SDL.h>

typedef enum {
    MOTION_STATIC,
    MOTION_SPIN,
    MOTION_DRIFT,
    MOTION_PULSE,
    MOTION_BOUNCE,
    MOTION_WAVE,
    MOTION_ZIGZAG,
    MOTION_SWIRL,
    MOTION_NOISE,
} MotionType;

typedef enum {
    SHAPE_NONE = -1,
    SHAPE_SQUARE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE,
} ShapeType;

typedef enum {
    ENTITY_OBJECT,
    ENTITY_LINE,
} EntityType;

typedef struct {
    float x, y;
    float cx, cy;
    float size;
    SDL_Color color;
    MotionType motion;
    ShapeType type;
    float speed;
    float radius;
} ArtShape;

typedef struct {
    float x1, x2, y1, y2;
    float thickness;
    SDL_Color color;
} ArtLine;

typedef struct {
    int id;
    EntityType kind;
    union {
        ArtShape shape;
        ArtLine line;
    };
} ArtEntity;

char* Motion2Str(MotionType motion);
char* ShapeType2Str(ShapeType obj);
ShapeType ParseShapeType(const char* str);
MotionType ParseMotion(const char* s);
SDL_Color ParseHexColor(const char* hex);


void ShapeUpdate(ArtShape* o, float time);
void ShapePaint(ArtShape* o, View* view);
void ShapePrint(ArtEntity* e);

void LinePaint(ArtLine* l, View* view);

void EntityPaint(ArtEntity* e, View* view);
void EntityUpdate(ArtEntity* e, float time);

#endif // ART_OBJECT_H

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
    OBJECT_NONE = -1,
    OBJECT_SQUARE,
    OBJECT_CIRCLE
} ObjectType;

typedef struct {
    float x, y;
    float cx, cy;
    float size;
    SDL_Color color;
    MotionType motion;
    ObjectType type;
    float speed;
    float radius;
} ArtObject;

void ObjectUpdate(ArtObject* o, float time);
void ObjectPaint(ArtObject* o, View* view);

#endif // ART_OBJECT_H

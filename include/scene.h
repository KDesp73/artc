#ifndef PARSER_H
#define PARSER_H

#include "art-object.h"

#define MAX_OBJECTS 128

typedef struct {
    int width;
    int height;
    SDL_Color background;
} SceneOptions;

typedef struct {
    SceneOptions options;

    ArtObject objects[MAX_OBJECTS];
    int count;
} Scene;

Scene SceneLoad(const char* filename);

#endif // PARSER_H


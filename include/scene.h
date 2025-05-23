#ifndef PARSER_H
#define PARSER_H

#include "entities.h"

#define MAX_ENTITIES 4096

typedef struct {
    int width;
    int height;
    SDL_Color background;
} SceneOptions;

typedef struct {
    SceneOptions options;

    ArtEntity entities[MAX_ENTITIES];
    int count;
    int next_id;
    bool loaded;
} Scene;

Scene SceneLoadArt(const char* filename);
Scene SceneLoadLua(const char* filename, bool sandbox);



#endif // PARSER_H


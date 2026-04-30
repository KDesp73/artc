#ifndef PARSER_H
#define PARSER_H

#include "SDL2/SDL_image.h"
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
Scene SceneLoadLua(const char* filename, bool sandbox, int script_argc, char** script_argv);

#endif // PARSER_H


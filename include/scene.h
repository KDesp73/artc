#ifndef PARSER_H
#define PARSER_H

#include "art-object.h"

#define MAX_OBJECTS 256

typedef struct {
    int width;
    int height;
    SDL_Color background;
} SceneOptions;

typedef struct {
    SceneOptions options;

    ArtObject objects[MAX_OBJECTS];
    int count;
    int next_id;
    bool loaded;
} Scene;

Scene SceneLoad(const char* filename);
Scene SceneLoadArt(const char* filename);
Scene SceneLoadLua(const char* filename);

// PARSING UTILS
ObjectType parse_type(const char* s);
MotionType parse_motion(const char* s);
SDL_Color parse_color(const char* hex);


#endif // PARSER_H


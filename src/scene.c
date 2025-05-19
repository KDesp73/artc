#include "scene.h"
#include "art-object.h"
#include "files.h"
#include <string.h>

Scene SceneLoad(const char* filename)
{
    if(!strcmp(file_extension(filename), "art")) return SceneLoadArt(filename);
    else return SceneLoadLua(filename);
}


// PARSING UTILS
MotionType parse_motion(const char* s)
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

SDL_Color parse_color(const char* hex)
{
    SDL_Color color = {255, 255, 255, 255};

    if (!hex || hex[0] != '#' || strlen(hex) != 7) {
        return color;
    }

    char r[3] = { hex[1], hex[2], '\0' };
    char g[3] = { hex[3], hex[4], '\0' };
    char b[3] = { hex[5], hex[6], '\0' };

    color.r = (Uint8)strtol(r, NULL, 16);
    color.g = (Uint8)strtol(g, NULL, 16);
    color.b = (Uint8)strtol(b, NULL, 16);

    return color;
}

ObjectType parse_type(const char* s)
{
    if(strcmp(s, "square") == 0) return OBJECT_SQUARE;
    else if(strcmp(s, "circle") == 0) return OBJECT_CIRCLE;
    return OBJECT_NONE;
}


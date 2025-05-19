#include "scene.h"
#include "art-object.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static MotionType parse_motion(const char* s)
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

static SDL_Color parse_color(const char* hex) {
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

static ObjectType parse_type(const char* s)
{
    if(strcmp(s, "square") == 0) return OBJECT_SQUARE;
    else if(strcmp(s, "circle") == 0) return OBJECT_CIRCLE;
    return OBJECT_NONE;
}

Scene SceneLoad(const char* filename)
{
    Scene scene = {0};
    scene.options.width = 640;
    scene.options.height = 480;
    scene.options.background = (SDL_Color){0, 0, 0, 255};

    FILE* file = fopen(filename, "r");
    if (!file) return scene;

    char line[256];
    ArtObject* obj = NULL;
    char current_section[64] = "";

    while (fgets(line, sizeof(line), file)) {
        char* l = line;
        while (*l == ' ' || *l == '\t') l++;
        if (*l == '#' || *l == '\n') continue;

        if (*l == '[') {
            sscanf(l, "[%63[^]]", current_section);
            if (strncmp(current_section, "object", 6) == 0 && scene.count < MAX_OBJECTS) {
                obj = &scene.objects[scene.count++];
                obj->motion = MOTION_STATIC;
                obj->color = (SDL_Color){255, 255, 255, 255};
                obj->cx = scene.options.width / 2;
                obj->cy = scene.options.height / 2;
            } else {
                obj = NULL;
            }
        } else if (strchr(l, ':')) {
            char key[64], val[128];
            sscanf(l, "%63[^:]: %127[^\n]", key, val);

            if (strcmp(current_section, "options") == 0) {
                if (strcmp(key, "width") == 0)
                    scene.options.width = atoi(val);
                else if (strcmp(key, "height") == 0)
                    scene.options.height = atoi(val);
                else if (strcmp(key, "background") == 0)
                    scene.options.background = parse_color(val);
            } else if (obj) {
                if (strcmp(key, "x") == 0) {
                    obj->x = (strcmp(val, "center") == 0) ? scene.options.width / 2 :
                             (strcmp(val, "random") == 0) ? rand() % scene.options.width : atof(val);
                } else if (strcmp(key, "y") == 0) {
                    obj->y = (strcmp(val, "center") == 0) ? scene.options.height / 2 :
                             (strcmp(val, "random") == 0) ? rand() % scene.options.height : atof(val);
                } else if (strcmp(key, "size") == 0) {
                    obj->size = atof(val);
                } else if (strcmp(key, "color") == 0) {
                    obj->color = parse_color(val);
                } else if (strcmp(key, "motion") == 0) {
                    obj->motion = parse_motion(val);
                } else if (strcmp(key, "speed") == 0) {
                    obj->speed = atof(val);
                } else if (strcmp(key, "radius") == 0) {
                    obj->radius = atof(val);
                } else if (strcmp(key, "type") == 0) {
                    obj->type = parse_type(val);
                }
            }        }
    }

    fclose(file);
    return scene;
}


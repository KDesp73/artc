#include "scene.h"

Scene SceneLoadArt(const char* filename)
{
    Scene scene = {0};
    scene.options.width = 640;
    scene.options.height = 480;
    scene.options.background = (SDL_Color){0, 0, 0, 255};

    FILE* file = fopen(filename, "r");
    if (!file) return scene;

    char line[256];
    ArtEntity* entity = NULL;
    char current_section[64] = "";

    while (fgets(line, sizeof(line), file)) {
        char* l = line;
        while (*l == ' ' || *l == '\t') l++;
        if (*l == '#' || *l == '\n') continue;

        if (*l == '[') {
            sscanf(l, "[%63[^]]", current_section);
            if (strncmp(current_section, "object", 6) == 0 && scene.count < MAX_ENTITIES) {
                entity = &scene.entities[scene.count++];
                entity->object.motion = MOTION_STATIC;
                entity->object.color = (SDL_Color){255, 255, 255, 255};
                entity->object.cx = scene.options.width / 2;
                entity->object.cy = scene.options.height / 2;
            } else {
                entity = NULL;
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
            } else if (entity) {
                if (strcmp(key, "x") == 0) {
                    entity->object.x = (strcmp(val, "center") == 0) ? scene.options.width / 2 :
                             (strcmp(val, "random") == 0) ? rand() % scene.options.width : atof(val);
                } else if (strcmp(key, "y") == 0) {
                    entity->object.y = (strcmp(val, "center") == 0) ? scene.options.height / 2 :
                             (strcmp(val, "random") == 0) ? rand() % scene.options.height : atof(val);
                } else if (strcmp(key, "size") == 0) {
                    entity->object.size = atof(val);
                } else if (strcmp(key, "color") == 0) {
                    entity->object.color = parse_color(val);
                } else if (strcmp(key, "motion") == 0) {
                    entity->object.motion = parse_motion(val);
                } else if (strcmp(key, "speed") == 0) {
                    entity->object.speed = atof(val);
                } else if (strcmp(key, "radius") == 0) {
                    entity->object.radius = atof(val);
                } else if (strcmp(key, "type") == 0) {
                    entity->object.type = parse_type(val);
                }
            }        }
    }

    fclose(file);
    scene.loaded = true;
    return scene;
}

#include "entities.h"
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
                entity->shape.motion = MOTION_STATIC;
                entity->shape.color = (SDL_Color){255, 255, 255, 255};
                entity->shape.cx = scene.options.width / 2;
                entity->shape.cy = scene.options.height / 2;
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
                    scene.options.background = ParseHexColor(val);
            } else if (entity) {
                if (strcmp(key, "x") == 0) {
                    entity->shape.x = (strcmp(val, "center") == 0) ? scene.options.width / 2 :
                             (strcmp(val, "random") == 0) ? rand() % scene.options.width : atof(val);
                } else if (strcmp(key, "y") == 0) {
                    entity->shape.y = (strcmp(val, "center") == 0) ? scene.options.height / 2 :
                             (strcmp(val, "random") == 0) ? rand() % scene.options.height : atof(val);
                } else if (strcmp(key, "size") == 0) {
                    entity->shape.size = atof(val);
                } else if (strcmp(key, "color") == 0) {
                    entity->shape.color = ParseHexColor(val);
                } else if (strcmp(key, "motion") == 0) {
                    entity->shape.motion = ParseMotion(val);
                } else if (strcmp(key, "speed") == 0) {
                    entity->shape.speed = atof(val);
                } else if (strcmp(key, "radius") == 0) {
                    entity->shape.radius = atof(val);
                } else if (strcmp(key, "type") == 0) {
                    entity->shape.type = ParseShapeType(val);
                }
            }        }
    }

    fclose(file);
    scene.loaded = true;
    return scene;
}

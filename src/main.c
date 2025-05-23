#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cli.h"
#include "entities.h"
#include "export.h"
#include "files.h"
#include "io/ansi.h"
#include "lua/lua.h"
#include "scene.h"
#include "view.h"
#define CLI_IMPLEMENTATION
#include "io/cli.h"
#define LOGGING_IMPLEMENTATION
#include "io/logging.h"
#include <stdbool.h>


static volatile int keep_running = 1;
void handle_sigint(int sig) {
    (void)sig;

    ANSI_SHOW_CURSOR();
    keep_running = 0;
}


View view = {0};
Scene scene = {0};
CliValues values = {0};
int main(int argc, char** argv)
{
    signal(SIGINT, handle_sigint);

    CliValuesInit(&values);
    CliParse(&values, argc, argv);
    

    if(dir_exists(".artc")) {
        dir_remove(".artc");
    }
    dir_create(".artc");

    if(!values.sandbox){
        WARN("Lua is not being sandboxed. With great power comes great responsibility");
    }

    if (argc == 1 || argv[argc - 1][0] == '-') {
        ERRO("Provide a file");
        return 1;
    }

    const char* file = argv[argc - 1];
    if (!strstr(file, ".art") && !strstr(file, ".lua")) {
        ERRO("The file must have an .art or a .lua extension");
        return 1;
    }
    bool is_art = strstr(file, ".art");
    values.output = (values.output) ? values.output : swap_ext(file, values.format);

    if(!strcmp(file_extension(file), "art"))
        scene = SceneLoadArt(file);
    else 
        scene = SceneLoadLua(file, values.sandbox);

    if(!scene.loaded) {
        ERRO("Could not load scene");
        return 1;
    }

    view.width = scene.options.width;
    view.height = scene.options.height;
    if (!ViewInit(&view)) return 1;

    if(values.ascii){ 
        values.export = false;
        view.fps = 30;
    }

    bool running = true;
    SDL_Event event;
    int frame = 0;

    const int frame_delay = 1000 / view.fps;
    Uint32 prev_time = SDL_GetTicks();
    Uint32 start_time = SDL_GetTicks();
    while (running && keep_running) {
        Uint32 frame_start = SDL_GetTicks();
        float delta_time = (frame_start - prev_time) / 1000.0f;
        prev_time = frame_start;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        float t = frame * 0.05f;

        SDL_SetRenderDrawColor(view.renderer,
                scene.options.background.r,
                scene.options.background.g,
                scene.options.background.b, 255);
        SDL_FillRect(view.surface, NULL, SDL_MapRGBA(view.surface->format,
                    scene.options.background.r,
                    scene.options.background.g,
                    scene.options.background.b, 255));

        if (!is_art) {
            lua_getglobal(view.L, "update");
            if (lua_isfunction(view.L, -1)) {
                lua_pushnumber(view.L, delta_time);
                if (lua_pcall(view.L, 1, 0, 0) != LUA_OK) {
                    ERRO("Lua update error: %s", lua_tostring(view.L, -1));
                    lua_pop(view.L, 1);
                    return 1;
                }
            } else {
                lua_pop(view.L, 1);
            }
        }

        for (int i = 0; i < scene.count; i++) {
            ArtEntity* e = &scene.entities[i];
            EntityUpdate(e, t);
            EntityPaint(e, &view);
        }

        if (values.ascii)
            ViewRenderAscii(&view);
        else
            ViewRender(&view);

        if (values.export) {
            char ppm_file[64];
            snprintf(ppm_file, sizeof(ppm_file), ".artc/frame%04d.ppm", frame);
            SaveFrameToPPM(ppm_file, view.width, view.height, view.surface);
        }

        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_delay > frame_time) {
            SDL_Delay(frame_delay - frame_time);
        }

        frame++;

        if (values.durations_s > 0) {
            Uint32 elapsed_ms = SDL_GetTicks() - start_time;
            if (elapsed_ms >= values.durations_s * 1000) {
                running = false;
            }
        }
    }

    if(values.export)
        Export(values.format, values.output, view.fps);

cleanup:
    ViewFree(&view);

    return 0;
}

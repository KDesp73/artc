#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
void handle_exit(int sig) {
    (void)sig;
    ANSI_SHOW_CURSOR();
    keep_running = 0;
}

View view = {0};
Scene scene = {0};
CliValues values = {0};

int main(int argc, char** argv)
{
    // Setup signal handlers
    signal(SIGINT, handle_exit);
    signal(SIGKILL, handle_exit);
    signal(SIGTERM, handle_exit);
    signal(SIGQUIT, handle_exit);

    // Initialize and parse CLI
    CliValuesInit(&values);
    CliParse(&values, argc, argv);

    // Setup working directory
    if (dir_exists(".artc")) {
        dir_remove(".artc");
    }
    dir_create(".artc");

    // Security warning
    if (!values.sandbox) {
        WARN("Lua is not being sandboxed. With great power comes great responsibility");
    }

    // Validate input
    if (argc == 1 || argv[argc - 1][0] == '-') {
        ERRO("Provide a file");
        return 1;
    }

    const char* file = argv[argc - 1];
    if (!strstr(file, ".art") && !strstr(file, ".lua")) {
        ERRO("The file must have an .art or a .lua extension");
        return 1;
    }
    if (!file_exists(file)) {
        ERRO("File %s does not exist", file);
        return 1;
    }

    // Load scene
    bool is_art = strstr(file, ".art");
    values.output = (values.output) ? values.output : swap_ext(file, values.format);

    if (!strcmp(file_extension(file), "art"))
        scene = SceneLoadArt(file);
    else 
        scene = SceneLoadLua(file, values.sandbox);

    if (!scene.loaded) {
        ERRO("Could not load scene");
        return 1;
    }

    // Initialize view
    view.width = scene.options.width;
    view.height = scene.options.height;
    if (!ViewInit(&view)) {
        PANIC("Unexpected error");
    }

    if (values.ascii) values.export = false;
    if (!strcmp(values.format, "png")) values.durations_s = 1;

    // Main loop variables
    bool running = true;
    SDL_Event event;
    const float fixed_time_step = 1.0f / view.fps;
    const int frame_delay = 1000 / view.fps;
    int frames_exported = 0;
    int total_frames = (values.durations_s > 0) ? (int)(view.fps * values.durations_s) : -1;
    Uint32 frame_start = SDL_GetTicks();
    float accumulated_time = 0;

    // Main render loop
    while (running && keep_running && (total_frames < 0 || frames_exported < total_frames)) {
        Uint32 current_time = SDL_GetTicks();
        float real_delta = (current_time - frame_start) / 1000.0f;
        frame_start = current_time;
        accumulated_time += real_delta;

        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        // Fixed timestep updates
        while (accumulated_time >= fixed_time_step) {
            float t = frames_exported * fixed_time_step;

            // Clear screen
            SDL_SetRenderDrawColor(view.renderer,
                                 scene.options.background.r,
                                 scene.options.background.g,
                                 scene.options.background.b, 255);
            SDL_RenderClear(view.renderer);

            // Lua updates
            if (!is_art) {
                lua_getglobal(view.L, "update");
                if (lua_isfunction(view.L, -1)) {
                    lua_pushnumber(view.L, fixed_time_step);
                    if (lua_pcall(view.L, 1, 0, 0) != LUA_OK) {
                        ERRO("Lua update error: %s", lua_tostring(view.L, -1));
                        lua_pop(view.L, 1);
                        return 1;
                    }
                } else {
                    lua_pop(view.L, 1);
                }
            }

            // Render entities
            for (int i = 0; i < scene.count; i++) {
                ArtEntity* e = &scene.entities[i];
                EntityUpdate(e, t);
                EntityPaint(e, &view);
            }

            // Export frame
            if (values.export) {
                char ppm_file[64];
                snprintf(ppm_file, sizeof(ppm_file), ".artc/frame%04d.ppm", frames_exported);
                SaveFrameToPPM(ppm_file, &view);
                frames_exported++;
            }

            accumulated_time -= fixed_time_step;
        }

        // Only present if not in headless export mode
        if (!values.export /*|| !values.headless*/) {
            if (values.ascii) {
                ViewRenderAscii(&view);
            } else {
                SDL_RenderPresent(view.renderer);
            }
        }

        // Frame rate limiting
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_delay > frame_time) {
            SDL_Delay(frame_delay - frame_time);
        }
    }

    // Final export if needed
    if (values.export) {
        Export(values.format, values.output, view.fps);
    }

    // Cleanup
    ViewFree(&view);
    return 0;
}

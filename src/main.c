#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "art-object.h"
#include "export.h"
#include "files.h"
#include "help.h"
#include "lua/lua.h"
#include "scene.h"
#include "view.h"
#include "version.h"
#define CLI_IMPLEMENTATION
#include "io/cli.h"
#define LOGGING_IMPLEMENTATION
#include "io/logging.h"
#include <stdbool.h>

static bool validate_format(char* fmt)
{
    char* allowed[] = {
        "gif", "mp4",
    };
    for(size_t i = 0; i < sizeof(allowed)/sizeof(allowed[0]); i++){
        if(!strcmp(fmt, allowed[i])) return true;
    }

    return false;
}

View view = {0};
Scene scene = {0};
int main(int argc, char** argv)
{
    int maj, min, pat;
    version(&maj, &min, &pat);

    cli_args_t args = cli_args_make(
        cli_arg_new('h', "help", "", no_argument),
        cli_arg_new('v', "version", "", no_argument),
        cli_arg_new('x', "export", "", no_argument),
        cli_arg_new('F', "format", "", required_argument),
        cli_arg_new('o', "output", "", required_argument),
        NULL
    );

    // CLI argument values
    bool export = false;
    char* format = "mp4";
    char* output = NULL;

    int opt;
    LOOP_ARGS(opt, args) {
        switch (opt) {
            case 'h':
                help();
                exit(0);
            case 'v':
                printf("artc v%d.%d.%d\n", maj, min, pat);
                exit(0);
            case 'x':
                export = true;
                break;
            case 'F':
                format = optarg;
                if(!validate_format(format)) {
                    ERRO("Invalid format: %s", format);
                    exit(1);
                }

                break;
            case 'o':
                output = optarg;
                break;
            default:
                exit(1);
        }
    }
    cli_args_free(&args);
    printf("artc v%d.%d.%d\n", maj, min, pat);

    if(dir_exists(".artc")) {
        dir_remove(".artc");
    }
    dir_create(".artc");

    if (argc == 1 || argv[argc - 1][0] == '-') {
        ERRO("Provide an .art file");
        return 1;
    }

    const char* file = argv[argc - 1];
    if (!strstr(file, ".art") && !strstr(file, ".lua")) {
        ERRO("The file must have an .art or a .lua extension");
        return 1;
    }
    output = (output) ? output : swap_ext(file, format);

    scene = SceneLoad(file);
    if(!scene.loaded) {
        ERRO("Could not load scene");
        return 1;
    }

    view.width = scene.options.width;
    view.height = scene.options.height;
    if (!ViewInit(&view)) return 1;

    bool running = true;
    SDL_Event event;
    int frame = 0;

    const int target_fps = 30;
    const int frame_delay = 1000 / target_fps;
    Uint32 prev_time = SDL_GetTicks();
    while (running) {
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

        for (int i = 0; i < scene.count; i++) {
            ArtObject* o = &scene.objects[i];
            ObjectUpdate(o, t);
            ObjectPaint(o, &view);
        }
        // ObjectPrint(&scene.objects[0]);

        SDL_UpdateTexture(view.texture, NULL, view.surface->pixels, view.surface->pitch);
        SDL_RenderClear(view.renderer);
        SDL_RenderCopy(view.renderer, view.texture, NULL, NULL);
        SDL_RenderPresent(view.renderer);

        if(export) {
            // Save frame to PPM
            char ppm_file[64];
            snprintf(ppm_file, sizeof(ppm_file), ".artc/frame%04d.ppm", frame); // pad for ffmpeg
            save_frame_ppm(ppm_file, view.width, view.height, view.surface);
        }

        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_delay > frame_time) {
            SDL_Delay(frame_delay - frame_time);
        }
        frame++;
    }

    if(export){
        char command[256];
        if(strcmp(format, "mp4") == 0) {
            snprintf(command, 256, "ffmpeg -v quiet -framerate 30 -i .artc/frame%%04d.ppm -pix_fmt yuv420p %s", output);
            system(command);
        } else if(strcmp(format, "gif") == 0) {
            system("ffmpeg -v quiet -framerate 30 -i .artc/frame%04d.ppm -filter_complex \"[0:v] palettegen\" .artc/palette.png");
            snprintf(command, 256, "ffmpeg -v quiet -framerate 30 -i .artc/frame%%04d.ppm -i .artc/palette.png -filter_complex \"[0:v][1:v] paletteuse\" %s", output);
            system(command);
        }
    }

cleanup:
    ViewFree(&view);

    return 0;
}

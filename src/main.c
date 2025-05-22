#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "art-object.h"
#include "export.h"
#include "files.h"
#include "help.h"
#include "io/ansi.h"
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

static volatile int keep_running = 1;
void handle_sigint(int sig) {
    (void)sig;

    ANSI_SHOW_CURSOR();
    keep_running = 0;
}

View view = {0};
Scene scene = {0};
int main(int argc, char** argv)
{
    signal(SIGINT, handle_sigint);
    int maj, min, pat;
    version(&maj, &min, &pat);

    cli_args_t args = cli_args_make(
        cli_arg_new('h', "help", "", no_argument),
        cli_arg_new('v', "version", "", no_argument),
        cli_arg_new('x', "export", "", no_argument),
        cli_arg_new('F', "format", "", required_argument),
        cli_arg_new('o', "output", "", required_argument),
        cli_arg_new('A', "ascii", "", no_argument),
        cli_arg_new('S', "no-sandbox", "", no_argument),
        NULL
    );

    // CLI argument values
    bool export = false;
    char* format = "mp4";
    char* output = NULL;
    bool ascii = false;
    bool sandbox = true;

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
            case 'A':
                ascii = true;
                break;
            case 'S':
                sandbox = false;
                break;
            default:
                exit(1);
        }
    }
    cli_args_free(&args);

    if(dir_exists(".artc")) {
        dir_remove(".artc");
    }
    dir_create(".artc");

    if(!sandbox){
        WARN("Lua is not being sandboxed. With great power comes great responsibility");
    }

    if (argc == 1 || argv[argc - 1][0] == '-') {
        ERRO("Provide an .art file");
        return 1;
    }

    const char* file = argv[argc - 1];
    if (!strstr(file, ".art") && !strstr(file, ".lua")) {
        ERRO("The file must have an .art or a .lua extension");
        return 1;
    }
    bool is_art = strstr(file, ".art");
    output = (output) ? output : swap_ext(file, format);

    if(!strcmp(file_extension(file), "art"))
        scene = SceneLoadArt(file);
    else 
        scene = SceneLoadLua(file, sandbox);

    if(!scene.loaded) {
        ERRO("Could not load scene");
        return 1;
    }

    view.width = scene.options.width;
    view.height = scene.options.height;
    if (!ViewInit(&view)) return 1;

    if(ascii){ 
        export = false;
        view.fps = 30;
    }

    bool running = true;
    SDL_Event event;
    int frame = 0;

    const int frame_delay = 1000 / view.fps;
    Uint32 prev_time = SDL_GetTicks();
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

        if(!is_art){
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
            ArtObject* o = &scene.objects[i];
            ObjectUpdate(o, t);
            ObjectPaint(o, &view);
        }

        if(ascii)
            ViewRenderAscii(&view);
        else
            ViewRender(&view);

        if(export) {
            // Save frame to PPM
            char ppm_file[64];
            snprintf(ppm_file, sizeof(ppm_file), ".artc/frame%04d.ppm", frame); // pad for ffmpeg
            SaveFrameToPPM(ppm_file, view.width, view.height, view.surface);
        }

        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_delay > frame_time) {
            SDL_Delay(frame_delay - frame_time);
        }
        frame++;
    }

    if(export)
        Export(format, output, view.fps);

cleanup:
    ViewFree(&view);

    return 0;
}

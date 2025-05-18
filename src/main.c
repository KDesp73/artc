#include <stdio.h>
#include <stdlib.h>
#include "art-object.h"
#include "export.h"
#include "scene.h"
#include "view.h"
#include "version.h"
#define CLI_IMPLEMENTATION
#include "io/cli.h"
#define LOGGING_IMPLEMENTATION
#include "io/logging.h"
#include "SDL2/SDL.h"
#include <stdbool.h>


int main(int argc, char** argv)
{
    srand(73);

    int maj, min, pat;
    version(&maj, &min, &pat);

    cli_args_t args = cli_args_make(
        cli_arg_new('h', "help", "", no_argument),
        cli_arg_new('v', "version", "", no_argument),
        cli_arg_new('x', "export", "", no_argument),
        cli_arg_new('o', "output", "", required_argument),
        cli_arg_new('F', "format", "", required_argument),
        NULL
    );

    // CLI argument values
    bool export = false;
    const char* output = NULL;
    const char* format = "mp4";

    int opt;
    LOOP_ARGS(opt, args) {
        switch (opt) {
            case 'h':
                // TODO: Help
                exit(0);
            case 'v':
                printf("artc v%d.%d.%d\n", maj, min, pat);
                exit(0);
            case 'x':
                export = true;
                break;
            case 'o':
                output = optarg;
                break;
            case 'F':
                format = optarg;
                break;
            default:
                exit(1);
        }
    }
    cli_args_free(&args);
    printf("artc v%d.%d.%d\n", maj, min, pat);

    if (argv[argc - 1][0] == '-') {
        ERRO("Provide a .art file at the end of the arguments");
        return 1;
    }

    const char* file = argv[argc - 1];
    if (!strstr(file, ".art")) {
        ERRO("The file must have a .art extension");
        return 1;
    }

    Scene scene = SceneLoad(file);

    View view = {0};
    view.width = scene.options.width;
    view.height = scene.options.height;
    ViewInit(&view);

    bool running = true;
    SDL_Event event;
    int frame = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        float t = frame * 0.05f; // Time variable for motion

        // Clear screen with background color
        SDL_SetRenderDrawColor(view.renderer,
                scene.options.background.r,
                scene.options.background.g,
                scene.options.background.b, 255);
        SDL_RenderClear(view.renderer);

        // Update and draw each object
        for (int i = 0; i < scene.count; i++) {
            ArtObject* o = &scene.objects[i];
            ObjectUpdate(o, t);
            ObjectPaint(o, &view);
        }

        SDL_RenderPresent(view.renderer);

        if(export) {
            // Save frame to PPM
            char ppm_file[64];
            snprintf(ppm_file, sizeof(ppm_file), "frame%04d.ppm", frame); // pad for ffmpeg
            save_frame_ppm(ppm_file, view.width, view.height, view.renderer);
        }

        SDL_Delay(33); // ~30 FPS
        frame++;
    }

    if(export){
        char command[256];
        if(strcmp(format, "mp4") == 0) {
            snprintf(command, 256, "ffmpeg -v quiet -framerate 30 -i frame%%04d.ppm -pix_fmt yuv420p %s", output);
            system(command);
        } else if(strcmp(format, "gif") == 0) {
            system("ffmpeg -framerate 30 -i frame%04d.ppm -filter_complex \"[0:v] palettegen\" palette.png");
            snprintf(command, 256, "ffmpeg -framerate 30 -i frame%%04d.ppm -i palette.png -filter_complex \"[0:v][1:v] paletteuse\" %s", output);
            system(command);
        }
    }

cleanup:
    ViewFree(&view);
    SDL_Quit();


    return 0;
}

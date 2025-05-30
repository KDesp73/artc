#include "export.h"
#include <SDL2/SDL_render.h>

void Export(const char* format, const char* output, size_t fps)
{
    char command[256];
    if(strcmp(format, "mp4") == 0) {
        snprintf(command, 256, "ffmpeg -v quiet -framerate %zu -i .artc/frame%%04d.ppm -pix_fmt yuv420p %s", fps, output);
        system(command);
    } else if(strcmp(format, "gif") == 0) {
        system("ffmpeg -v quiet -framerate 30 -i .artc/frame%04d.ppm -filter_complex \"[0:v] palettegen\" .artc/palette.png");
        snprintf(command, 256, "ffmpeg -v quiet -framerate %zu -i .artc/frame%%04d.ppm -i .artc/palette.png -filter_complex \"[0:v][1:v] paletteuse\" %s", fps, output);
        system(command);
    }

}

void SaveFrameToPPM(const char* filename, View* view)
{
    if (!filename || !view || !view->renderer) return;

    int width, height;
    SDL_GetRendererOutputSize(view->renderer, &width, &height);

    Uint8* pixels = malloc(width * height * 4);
    if (!pixels) return;

    if (SDL_RenderReadPixels(view->renderer, NULL, SDL_PIXELFORMAT_RGBA8888, pixels, width * 4) != 0) {
        fprintf(stderr, "SDL_RenderReadPixels failed: %s\n", SDL_GetError());
        free(pixels);
        return;
    }

    FILE* f = fopen(filename, "wb");
    if (!f) {
        free(pixels);
        return;
    }

    fprintf(f, "P6\n%d %d\n255\n", width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Uint8* p = pixels + y * width * 4 + x * 4;
            fwrite(&p[0], 1, 1, f); // R
            fwrite(&p[1], 1, 1, f); // G
            fwrite(&p[2], 1, 1, f); // B
        }
    }

    fclose(f);
    free(pixels);
}

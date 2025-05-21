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

void SaveFrameToPPM(const char* filename, int width, int height, SDL_Surface* surface)
{
    FILE* f = fopen(filename, "wb");
    if (!f) return;

    fprintf(f, "P6\n%d %d\n255\n", width, height);

    Uint8* pixels = (Uint8*)surface->pixels;
    int pitch = surface->pitch;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Uint8* p = pixels + y * pitch + x * 4;
            Uint8 r = p[0];
            Uint8 g = p[1];
            Uint8 b = p[2];
            fwrite(&r, 1, 1, f);
            fwrite(&g, 1, 1, f);
            fwrite(&b, 1, 1, f);
        }
    }

    fclose(f);
}

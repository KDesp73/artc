#include "export.h"
#include <SDL2/SDL_render.h>

void save_frame_ppm(const char* filename, int width, int height, SDL_Surface* surface)
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

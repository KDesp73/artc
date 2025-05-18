#include "export.h"
#include <SDL2/SDL_render.h>

void save_frame_ppm(const char* filename, int width, int height, SDL_Renderer* renderer)
{
    Uint32* pixels = malloc(width * height * sizeof(Uint32));
    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, pixels, width * sizeof(Uint32));

    FILE* f = fopen(filename, "wb");
    fprintf(f, "P6\n%d %d\n255\n", width, height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Uint32 pixel = pixels[y * width + x];
            Uint8 r = (pixel >> 16) & 0xff;
            Uint8 g = (pixel >> 8) & 0xff;
            Uint8 b = pixel & 0xff;
            fwrite(&r, 1, 1, f);
            fwrite(&g, 1, 1, f);
            fwrite(&b, 1, 1, f);
        }
    }

    fclose(f);
    free(pixels);
}

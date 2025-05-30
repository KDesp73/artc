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

/**
 * Saves the current renderer contents as a PPM file
 * 
 * @param renderer The SDL_Renderer to capture
 * @param filename The output PPM filename
 * @return 0 on success, -1 on error
 */
static int saveRendererToPPM(SDL_Renderer* renderer, const char* filename)
{
    if (!renderer || !filename) {
        return -1;
    }

    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);
    
    // Create a temporary surface to hold the pixel data
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(
        0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
    
    if (!surface) {
        fprintf(stderr, "Could not create surface: %s\n", SDL_GetError());
        return -1;
    }
    
    if (SDL_RenderReadPixels(renderer, NULL, surface->format->format,
                            surface->pixels, surface->pitch) != 0) {
        fprintf(stderr, "Could not read pixels: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return -1;
    }
    
    FILE* file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Could not open file for writing: %s\n", filename);
        SDL_FreeSurface(surface);
        return -1;
    }
    
    fprintf(file, "P6\n%d %d\n255\n", width, height);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Uint32 pixel = ((Uint32*)surface->pixels)[y * width + x];
            Uint8 r = (pixel >> surface->format->Rshift) & 0xFF;
            Uint8 g = (pixel >> surface->format->Gshift) & 0xFF;
            Uint8 b = (pixel >> surface->format->Bshift) & 0xFF;
            
            fwrite(&r, 1, 1, file);
            fwrite(&g, 1, 1, file);
            fwrite(&b, 1, 1, file);
        }
    }
    
    fclose(file);
    SDL_FreeSurface(surface);
    
    return 0;
}

void SaveFrameToPPM(const char* filename, View* view)
{
    saveRendererToPPM(view->renderer, filename);
}

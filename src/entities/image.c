#include "entities.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

static void ApplyNegativeFilter(SDL_Surface* surface)
{
    if (!surface) return;

    SDL_LockSurface(surface);

    Uint8* pixels = (Uint8*)surface->pixels;
    int bpp = surface->format->BytesPerPixel;

    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            Uint8* p = pixels + y * surface->pitch + x * bpp;

            Uint8 r, g, b, a;
            SDL_GetRGBA(*(Uint32*)p, surface->format, &r, &g, &b, &a);

            r = 255 - r;
            g = 255 - g;
            b = 255 - b;

            *(Uint32*)p = SDL_MapRGBA(surface->format, r, g, b, a);
        }
    }

    SDL_UnlockSurface(surface);
}

static void ApplyGreyscaleFilter(SDL_Surface* surface)
{
    if (!surface) return;

    SDL_LockSurface(surface);

    Uint8* pixels = (Uint8*)surface->pixels;
    int bpp = surface->format->BytesPerPixel;

    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            Uint8* p = pixels + y * surface->pitch + x * bpp;

            Uint8 r, g, b, a;
            SDL_GetRGBA(*(Uint32*)p, surface->format, &r, &g, &b, &a);

            Uint8 grey = (Uint8)(0.3 * r + 0.59 * g + 0.11 * b);

            *(Uint32*)p = SDL_MapRGBA(surface->format, grey, grey, grey, a);
        }
    }

    SDL_UnlockSurface(surface);
}


void ImagePaint(ArtImage* i, View* view)
{
    if (!i || !view || !i->src) return;

    SDL_Surface* image = IMG_Load(i->src);
    if (!image) {
        SDL_Log("Failed to load image %s: %s", i->src, IMG_GetError());
        return;
    }

    if (i->filter) {
        if (strcmp(i->filter, "negative") == 0) {
            ApplyNegativeFilter(image);
        } else if (strcmp(i->filter, "greyscale") == 0 || strcmp(i->filter, "grayscale") == 0) {
            ApplyGreyscaleFilter(image);
        }
    }

    SDL_Rect dest = {
        (int)i->x,
        (int)i->y,
        (int)i->w,
        (int)i->h
    };

    SDL_BlitScaled(image, NULL, view->surface, &dest);

    SDL_FreeSurface(image);
}

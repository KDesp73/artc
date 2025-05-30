#include "entities.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

static void ApplyChannelSwap(SDL_Surface* surface)
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

            *(Uint32*)p = SDL_MapRGBA(surface->format, b, g, r, a);
        }
    }

    SDL_UnlockSurface(surface);
}

static void ApplyInvertRedFilter(SDL_Surface* surface)
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

            *(Uint32*)p = SDL_MapRGBA(surface->format, r, g, b, a);
        }
    }

    SDL_UnlockSurface(surface);
}

static void ApplyInvertGreenFilter(SDL_Surface* surface)
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

            g = 255 - g;

            *(Uint32*)p = SDL_MapRGBA(surface->format, r, g, b, a);
        }
    }

    SDL_UnlockSurface(surface);
}

static void ApplyInvertBlueFilter(SDL_Surface* surface)
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

            b = 255 - b;

            *(Uint32*)p = SDL_MapRGBA(surface->format, r, g, b, a);
        }
    }

    SDL_UnlockSurface(surface);
}


static void ApplyThresholdFilter(SDL_Surface* surface, Uint8 threshold)
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

            Uint8 brightness = (Uint8)(0.3 * r + 0.59 * g + 0.11 * b);
            Uint8 bw = brightness > threshold ? 255 : 0;

            *(Uint32*)p = SDL_MapRGBA(surface->format, bw, bw, bw, a);
        }
    }

    SDL_UnlockSurface(surface);
}

static void ApplySepiaFilter(SDL_Surface* surface)
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

            Uint8 tr = (Uint8)SDL_min(255, 0.393 * r + 0.769 * g + 0.189 * b);
            Uint8 tg = (Uint8)SDL_min(255, 0.349 * r + 0.686 * g + 0.168 * b);
            Uint8 tb = (Uint8)SDL_min(255, 0.272 * r + 0.534 * g + 0.131 * b);

            *(Uint32*)p = SDL_MapRGBA(surface->format, tr, tg, tb, a);
        }
    }

    SDL_UnlockSurface(surface);
}

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
    if (!i || !view || !view->renderer || !i->src) return;

    SDL_Surface* image = IMG_Load(i->src);
    if (!image) {
        SDL_Log("Failed to load image %s: %s", i->src, IMG_GetError());
        return;
    }

    // Apply filters on the surface as before
    if (i->filter) {
        if (strcmp(i->filter, "negative") == 0) {
            ApplyNegativeFilter(image);
        } else if (strcmp(i->filter, "greyscale") == 0 || strcmp(i->filter, "grayscale") == 0) {
            ApplyGreyscaleFilter(image);
        } else if (strcmp(i->filter, "sepia") == 0) {
            ApplySepiaFilter(image);
        } else if (strcmp(i->filter, "threshold") == 0) {
            ApplyThresholdFilter(image, 128);
        } else if (strcmp(i->filter, "invert-red") == 0) {
            ApplyInvertRedFilter(image);
        } else if (strcmp(i->filter, "invert-green") == 0) {
            ApplyInvertGreenFilter(image);
        } else if (strcmp(i->filter, "invert-blue") == 0) {
            ApplyInvertBlueFilter(image);
        } else if (strcmp(i->filter, "channel-swap") == 0) {
            ApplyChannelSwap(image);
        }
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(view->renderer, image);
    if (!texture) {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        SDL_FreeSurface(image);
        return;
    }

    SDL_Rect dest = {
        (int)i->x,
        (int)i->y,
        (int)i->w,
        (int)i->h
    };

    SDL_RenderCopy(view->renderer, texture, NULL, &dest);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(image);
}

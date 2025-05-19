#include "view.h"
#include <SDL2/SDL.h>
#include <string.h>
#include "io/logging.h"
#include "scene.h"

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600

bool ViewInit(View* view)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        ERRO("SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }

    // Default values
    if(view->width == 0) view->width = DEFAULT_WIDTH;
    if(view->height == 0) view->height = DEFAULT_HEIGHT;
    if (view->title[0] == '\0') strncpy(view->title, "artc", sizeof(view->title));
    if(strlen(view->title) == 0) strncpy(view->title, "artc", sizeof(view->title));

    view->window = SDL_CreateWindow(view->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, view->width, view->height, SDL_WINDOW_SHOWN);
    if (!view->window) {
        ERRO("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    view->renderer = SDL_CreateRenderer(view->window, -1, SDL_RENDERER_ACCELERATED);
    if (!view->renderer) {
        ERRO("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(view->window);
        SDL_Quit();
        return false;
    }

    view->surface = SDL_CreateRGBSurfaceWithFormat(0, view->width, view->height, 32, SDL_PIXELFORMAT_RGBA32);
    if (!view->surface) {
        ERRO("SDL_CreateRGBSurfaceWithFormat Error: %s\n", SDL_GetError());
        ViewFree(view);
        return false;
    }

    view->texture = SDL_CreateTexture(view->renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, view->width, view->height);
    if (!view->texture) {
        ERRO("SDL_CreateTexture Error: %s\n", SDL_GetError());
        ViewFree(view);
        return false;
    }

    return true;
}

void ViewFree(View* view)
{
    if (view->texture) SDL_DestroyTexture(view->texture);
    if (view->surface) SDL_FreeSurface(view->surface);
    if (view->renderer) SDL_DestroyRenderer(view->renderer);
    if (view->window) SDL_DestroyWindow(view->window);
    SDL_Quit();
    lua_close(view->L);
}


#include "view.h"
#include <SDL2/SDL.h>
#include <string.h>
#include "io/logging.h"

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
    if(strlen(view->title) == 0) strncpy(view->title, "artc", 64);

    view->window = SDL_CreateWindow(view->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, view->width, view->height, SDL_WINDOW_SHOWN);
    if (!view->window) {
        ERRO("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    view->renderer = SDL_CreateRenderer(view->window, -1, SDL_RENDERER_ACCELERATED);
    if (!view) {
        ERRO("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(view->window);
        SDL_Quit();
        return false;
    }
    
    return true;
}

void ViewFree(View* view)
{
    SDL_DestroyRenderer(view->renderer);
    SDL_DestroyWindow(view->window);
}

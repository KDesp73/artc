#ifndef RENDERER_H
#define RENDERER_H

#include "lua/lua.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>

typedef struct {
   SDL_Window* window;
   SDL_Renderer* renderer;
   SDL_Surface* surface;
   SDL_Texture* texture;
   lua_State* L;

   size_t width;
   size_t height;
   char title[64];
} View;

bool ViewInit(View* view);
void ViewFree(View* view);

extern View view;

#endif // RENDERER_H

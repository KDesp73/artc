#include "SDL2/SDL_ttf.h"
#include "assets/SuperFunky-lgmWw.h"
#include "entities.h"
#include "assets/CozetteCrossedSevenVector.h"


TTF_Font* LoadFontFromMemory(const char* name, int size)
{
    if (strcmp(name, "artc:cozette") == 0) {
        SDL_RWops* rw = SDL_RWFromConstMem(__assets_CozetteCrossedSevenVector_ttf, __assets_CozetteCrossedSevenVector_ttf_len);
        if (!rw) return NULL;
        return TTF_OpenFontRW(rw, 1, size);
    } else if (strcmp(name, "artc:funky") == 0) {
        SDL_RWops* rw = SDL_RWFromConstMem(__assets_SuperFunky_lgmWw_ttf, __assets_SuperFunky_lgmWw_ttf_len);
        if (!rw) return NULL;
        return TTF_OpenFontRW(rw, 1, size);
    }
    return NULL;
}

void TextPaint(ArtText* t, View* view)
{
    if (!t || !t->content || !t->font) return;

    TTF_Font* font = LoadFontFromMemory(t->font, t->font_size);
    if(!font) font = TTF_OpenFont(t->font, (int)t->font_size);
    if (!font) {
        fprintf(stderr, "Failed to load font %s: %s\n", t->font, TTF_GetError());
        return;
    }

    SDL_Surface* text_surface = TTF_RenderText_Shaded(font, t->content, t->fg, t->bg);
    if (!text_surface) {
        fprintf(stderr, "Failed to render text: %s\n", TTF_GetError());
        TTF_CloseFont(font);
        return;
    }

    SDL_Rect dst;
    dst.x = (int)t->x;
    dst.y = (int)t->y;
    dst.w = text_surface->w;
    dst.h = text_surface->h;

    SDL_BlitSurface(text_surface, NULL, view->surface, &dst);

    SDL_FreeSurface(text_surface);
    TTF_CloseFont(font);
}


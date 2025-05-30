#include "SDL2/SDL_ttf.h"
#include "assets/AovelSansRounded-rdDL.h"
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
    } else if (strcmp(name, "artc:aovel") == 0) {
        SDL_RWops* rw = SDL_RWFromConstMem(__assets_AovelSansRounded_rdDL_ttf, __assets_AovelSansRounded_rdDL_ttf_len);
        if (!rw) return NULL;
        return TTF_OpenFontRW(rw, 1, size);
    }
    return NULL;
}

void TextPaint(ArtText* t, View* view)
{
    if (!t || !t->content || !t->font || !view || !view->renderer) return;

    TTF_Font* font = LoadFontFromMemory(t->font, t->font_size);
    if (!font) font = TTF_OpenFont(t->font, (int)t->font_size);
    if (!font) {
        fprintf(stderr, "Failed to load font %s: %s\n", t->font, TTF_GetError());
        return;
    }

    SDL_Surface* text_surface = NULL;
    if (!t->blend) {
        text_surface = TTF_RenderText_Shaded(font, t->content, t->fg, t->bg);
    } else {
        text_surface = TTF_RenderText_Blended(font, t->content, t->fg);
    }
    
    if (!text_surface) {
        fprintf(stderr, "Failed to render text: %s\n", TTF_GetError());
        TTF_CloseFont(font);
        return;
    }

    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(view->renderer, text_surface);
    if (!text_texture) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(text_surface);
        TTF_CloseFont(font);
        return;
    }

    SDL_Rect dst = {
        .x = (int)t->x,
        .y = (int)t->y,
        .w = text_surface->w,
        .h = text_surface->h
    };

    SDL_RenderCopy(view->renderer, text_texture, NULL, &dst);

    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);
    TTF_CloseFont(font);
}


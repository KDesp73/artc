#include "SDL2/SDL_ttf.h"
#include "assets/AovelSansRounded-rdDL.h"
#include "assets/Movistar_Text_Regular.h"
#include "assets/SuperFunky-lgmWw.h"
#include "assets/Xfiles-5ynj.h"
#include "assets/fox_version_5_by_mickeyfan123_daxvfx5.h"
#include "assets/x-files.h"
#include "entities.h"
#include "assets/CozetteCrossedSevenVector.h"

#define FONT(_name, symbol) \
    if (strcmp(name, _name) == 0) { \
        SDL_RWops* rw = SDL_RWFromConstMem(symbol, symbol##_len); \
        if (!rw) return NULL; \
        return TTF_OpenFontRW(rw, 1, size); \
    }


TTF_Font* LoadFontFromMemory(const char* name, int size)
{
    FONT("artc:cozette", __assets_CozetteCrossedSevenVector_ttf)
    else FONT("artc:funky", __assets_SuperFunky_lgmWw_ttf)
    else FONT("artc:aovel", __assets_AovelSansRounded_rdDL_ttf)
    else FONT("artc:xfiles", __assets_xfiles1_x_files_ttf)
    else FONT("artc:xfiles-1", __assets_xfiles_font_Xfiles_5ynj_ttf)
    else FONT("artc:movistar", __assets_movistar_text_Movistar_Text_Regular_ttf)
    else FONT("artc:fox", __assets_fox_3_fox_version_5_by_mickeyfan123_daxvfx5_ttf)
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


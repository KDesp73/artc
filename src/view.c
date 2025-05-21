#include "view.h"
#include <SDL2/SDL.h>
#include <string.h>
#include "SDL2/SDL_surface.h"
#include "io/ansi.h"
#include "io/logging.h"
#include "scene.h"
#include <sys/ioctl.h>
#include <unistd.h>

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
    if(view->L)
        lua_close(view->L);
}


void ViewRender(View* view)
{
    SDL_UpdateTexture(view->texture, NULL, view->surface->pixels, view->surface->pitch);
    SDL_RenderClear(view->renderer);
    SDL_RenderCopy(view->renderer, view->texture, NULL, NULL);
    SDL_RenderPresent(view->renderer);
}

static Uint32 get_pixel(SDL_Surface* surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
        case 1: return *p;
        case 2: return *(Uint16*)p;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;
        case 4: return *(Uint32*)p;
        default: return 0;
    }
}

#define TERM_COLS 80
#define TERM_ROWS 24
static void get_terminal_size(int* rows, int* cols)
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        *cols = TERM_COLS;
        *rows = TERM_ROWS;
    } else {
        *cols = w.ws_col;
        *rows = w.ws_row;
    }
}

static void RenderSurfaceToTerminal(SDL_Surface* surface)
{
    const char* charset = " .:-=+*#%@";
    int charset_len = strlen(charset);

    int surf_w = surface->w;
    int surf_h = surface->h;
    int rows, cols;
    get_terminal_size(&rows, &cols);

    int px_per_col = surf_w / cols;
    int px_per_row = surf_h / rows;

    if (px_per_col < 1) px_per_col = 1;
    if (px_per_row < 1) px_per_row = 1;

    ANSI_HIDE_CURSOR();
    printf("\033[H\033[J");

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int x = col * px_per_col;
            int y = row * px_per_row;

            Uint32 pixel = get_pixel(surface, x, y);
            Uint8 r, g, b;
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);

            float brightness = (0.2126f * r + 0.7152f * g + 0.0722f * b) / 255.0f;
            int char_index = (int)(brightness * (charset_len - 1));
            char c = charset[char_index];

            printf("\033[38;2;%d;%d;%dm%c\033[0m", r, g, b, c);
        }
        printf("\n");
    }
    fflush(stdout);
}

void ViewRenderAscii(View* view)
{
    SDL_Surface* surface = view->surface;
    RenderSurfaceToTerminal(surface);
}

#include "entities.h"
#include <math.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_render.h"

void LinePaint(ArtLine* l, View* view)
{
    if (!l || !view || !view->renderer) return;

    SDL_Renderer* renderer = view->renderer;
    const Uint8 r = l->color.r;
    const Uint8 g = l->color.g;
    const Uint8 b = l->color.b;
    const Uint8 a = l->color.a;

    const float x1 = l->x1;
    const float y1 = l->y1;
    const float x2 = l->x2;
    const float y2 = l->y2;
    const float dx = x2 - x1;
    const float dy = y2 - y1;
    const float len_sq = dx * dx + dy * dy;
    if (len_sq < 1e-12f) {
        return;
    }

    const float length = sqrtf(len_sq);
    /* Default thickness 0: treat as a 1px line (avoids the old O(length)-per-stamp path). */
    const float t = l->thickness;
    if (t <= 1.0f) {
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        SDL_RenderDrawLineF(renderer, x1, y1, x2, y2);
        return;
    }

    /* Thick segment: one filled quad = 2 triangles (O(1) vs O(length * thickness²) points). */
    const float half_w = t * 0.5f;
    const float nx = (-dy / length) * half_w;
    const float ny = (dx / length) * half_w;

    SDL_Vertex v[4];
    for (int i = 0; i < 4; i++) {
        v[i].color.r = r;
        v[i].color.g = g;
        v[i].color.b = b;
        v[i].color.a = a;
        v[i].tex_coord.x = 0.0f;
        v[i].tex_coord.y = 0.0f;
    }

    v[0].position.x = x1 + nx;
    v[0].position.y = y1 + ny;
    v[1].position.x = x1 - nx;
    v[1].position.y = y1 - ny;
    v[2].position.x = x2 - nx;
    v[2].position.y = y2 - ny;
    v[3].position.x = x2 + nx;
    v[3].position.y = y2 + ny;

    const int indices[6] = { 0, 1, 2, 0, 2, 3 };
    SDL_RenderGeometry(renderer, NULL, v, 4, indices, 6);
}

#include "entities.h"
#include <math.h>
#include "SDL2/SDL.h"

static void draw_filled_circle(SDL_Renderer* renderer, int cx, int cy, int radius)
{
    int w, h;
    for (w = -radius; w <= radius; w++) {
        for (h = -radius; h <= radius; h++) {
            if (w * w + h * h <= radius * radius) {
                SDL_RenderDrawPoint(renderer, cx + w, cy + h);
            }
        }
    }
}

void LinePaint(ArtLine* l, View* view)
{
    if (!l || !view || !view->renderer) return;

    SDL_SetRenderDrawColor(view->renderer, l->color.r, l->color.g, l->color.b, l->color.a);

    float x1 = l->x1;
    float y1 = l->y1;
    float x2 = l->x2;
    float y2 = l->y2;
    float thickness = l->thickness;

    float dx = x2 - x1;
    float dy = y2 - y1;
    float length = sqrtf(dx * dx + dy * dy);
    if (length == 0) return;

    float half_thickness = thickness / 2.0f;

    int steps = (int)length;

    for (int i = 0; i <= steps; i++) {
        float t = (float)i / (float)steps;
        float cx = x1 + t * dx;
        float cy = y1 + t * dy;

        draw_filled_circle(view->renderer, (int)cx, (int)cy, (int)half_thickness);
    }
}

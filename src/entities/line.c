#include "entities.h"

void LinePaint(ArtLine* l, View* view)
{
    if (!l || !view || !view->surface) return;

    Uint32 color = SDL_MapRGBA(view->surface->format, l->color.r, l->color.g, l->color.b, l->color.a);

    Uint32* pixels = (Uint32*)view->surface->pixels;
    int pitch = view->surface->pitch / 4;

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

        int radius = (int)(half_thickness);

        int start_x = (int)(cx - radius);
        int end_x = (int)(cx + radius);
        int start_y = (int)(cy - radius);
        int end_y = (int)(cy + radius);

        for (int py = start_y; py <= end_y; py++) {
            if (py < 0 || py >= view->height) continue;

            for (int px = start_x; px <= end_x; px++) {
                if (px < 0 || px >= view->width) continue;

                int dxp = px - (int)cx;
                int dyp = py - (int)cy;

                if (dxp*dxp + dyp*dyp <= radius*radius) {
                    pixels[py * pitch + px] = color;
                }
            }
        }
    }
}


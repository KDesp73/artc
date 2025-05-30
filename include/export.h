#ifndef EXPORT_H
#define EXPORT_H

#include "SDL2/SDL_surface.h"
#include "view.h"

void Export(const char* format, const char* output, size_t fps);
void SaveFrameToPPM(const char* filename, View* view);

#endif // EXPORT_H

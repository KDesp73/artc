#ifndef EXPORT_H
#define EXPORT_H

#include "SDL2/SDL_surface.h"

void Export(const char* format, const char* output, size_t fps);
void SaveFrameToPPM(const char* filename, int width, int height, SDL_Surface* surface);

#endif // EXPORT_H

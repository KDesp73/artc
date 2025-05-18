#ifndef EXPORT_H
#define EXPORT_H

#include "scene.h"
#include "view.h"

void save_frame_ppm(const char* filename, int width, int height, SDL_Renderer* renderer);

#endif // EXPORT_H

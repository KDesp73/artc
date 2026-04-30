#ifndef EXPORT_H
#define EXPORT_H

#include "SDL2/SDL_surface.h"
#include "view.h"

/** Zero-pad width for exported frame filenames (ffmpeg image2 sequence). No practical cap on frame count. */
#define ARTC_EXPORT_FRAME_INDEX_WIDTH 9

void Export(const char* format, const char* output, size_t fps);
void SaveFrameToPPM(const char* filename, View* view);

#endif // EXPORT_H

#include "version.h"

void version(int* major, int* minor, int* patch)
{
    *major = VERSION_MAJOR;
    *minor = VERSION_MINOR;
    *patch = VERSION_PATCH;
}

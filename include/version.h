#ifndef VERSION_H
#define VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#define VERSION_MAJOR 0
#define VERSION_MINOR 5
#define VERSION_PATCH 2

#define VERSION_STRING "0.5.2"

#define VERSION_HEX ((VERSION_MAJOR * 10000) + (VERSION_MINOR * 100) + VERSION_PATCH)

/**
 * @brief Fills in the provided pointers with the current version numbers.
 * 
 * @param major Pointer to an int to store the major version
 * @param minor Pointer to an int to store the minor version
 * @param patch Pointer to an int to store the patch version
 */
static inline void version(int* major, int* minor, int* patch) {
    if (major) *major = VERSION_MAJOR;
    if (minor) *minor = VERSION_MINOR;
    if (patch) *patch = VERSION_PATCH;
}

#ifdef __cplusplus
}
#endif

#endif // VERSION_H

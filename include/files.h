#ifndef FILES_H
#define FILES_H

#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <string.h>

// Check if the path is a file
static inline bool is_file(const char* path) 
{
    struct stat st;
    return (stat(path, &st) == 0) && S_ISREG(st.st_mode);
}

// Check if the path is a directory
static inline bool is_directory(const char* path) 
{
    struct stat st;
    return (stat(path, &st) == 0) && S_ISDIR(st.st_mode);
}

// Get the file name without the extension
static inline char* file_name(const char* path)
{
    char* base = strrchr(path, '/');
    char* filename = base ? base + 1 : (char*)path; // Extract the actual filename

    char* dot = strrchr(filename, '.');
    size_t len = (dot && dot != filename) ? (size_t)(dot - filename) : strlen(filename);

    char* result = (char*)malloc(len + 1);
    if (!result) return NULL;
    strncpy(result, filename, len);
    result[len] = '\0';

    return result;
}

// Get the file extension (returns pointer within `path` or empty string)
static inline char* file_extension(const char* path)
{
    char* dot = strrchr(path, '.');
    return (dot && dot != path) ? dot + 1 : "";
}

static inline int file_delete(const char *filename) {
    if (remove(filename) == 0) {
        return true;
    } else {
        perror("File deletion failed");
        return false;
    }
}

static inline int dir_create(const char* path)
{
    struct stat st = {0};

    if (stat(path, &st) == -1) { // Check if the directory exists
        if (mkdir(path, 0777) != 0 && errno != EEXIST) { // Try to create it
            return 0;
        }
    }
    return 1;
}

static inline bool file_exists(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}


static inline bool file_write(const char* path, const char* content)
{
    FILE* __file = fopen(path, "w");
    if(!__file) {
        return false;
    }
    fprintf(__file, "%s", content);
    fclose(__file);

    return true;
}

static inline bool dir_exists(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0) && S_ISDIR(st.st_mode);
}

static inline int create_directories(const char *path)
{
    char temp[PATH_MAX];
    char *p = NULL;
    size_t len;

    // Copy path to a mutable string
    snprintf(temp, sizeof(temp), "%s", path);
    len = strlen(temp);

    // Remove trailing '/' if it exists
    if (temp[len - 1] == '/') {
        temp[len - 1] = '\0';
    }

    // Iterate over the path and create directories one by one
    for (p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0'; // Temporarily end the string here

            // Try creating the directory
            if (mkdir(temp, 0755) && errno != EEXIST) {
                perror("mkdir failed");
                return -1; // Failure
            }

            *p = '/'; // Restore slash
        }
    }

    // Final directory or file parent
    if (mkdir(temp, 0755) && errno != EEXIST) {
        perror("mkdir failed");
        return -1;
    }

    return 0; // Success
}

static inline int create_parent_directories(const char *path)
{
    char temp[PATH_MAX];
    char *p = NULL;

    // Copy path into a mutable buffer
    snprintf(temp, sizeof(temp), "%s", path);

    // Remove the file name to get only the directory path
    char *last_slash = strrchr(temp, '/');
    if (!last_slash) {
        // No slashes means there's no directory to create
        return 0;
    }
    *last_slash = '\0'; // Trim to parent directory path

    // Iterate over the path and create directories one by one
    for (p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0'; // Temporarily end string here

            // Try creating the directory
            if (mkdir(temp, 0755) && errno != EEXIST) {
                perror("mkdir failed");
                return -1;
            }

            *p = '/'; // Restore slash
        }
    }

    // Create the final directory in the path
    if (mkdir(temp, 0755) && errno != EEXIST) {
        perror("mkdir failed");
        return -1;
    }

    return 0; // Success
}

#endif // FILES_H

#include "preprocessor.h"
#include <magic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_URLS 64
#define MAX_URL_LEN 1024

// Download helper
static int download_file(const char* url, const char* output_path) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "curl -fsSL \"%s\" -o \"%s\"", url, output_path);
    return system(cmd);
}

static uint64_t hash(const void* data, size_t len)
{
    const uint8_t* bytes = (const uint8_t*)data;
    uint64_t hash = 14695981039346656037ULL; // FNV offset basis
    for (size_t i = 0; i < len; ++i) {
        hash ^= bytes[i];
        hash *= 1099511628211ULL; // FNV prime
    }
    return hash;
}

static int is_supported_image_ext(const char* ext) {
    return strcmp(ext, "png") == 0 || strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0 || strcmp(ext, "gif") == 0;
}

static const char* get_extension(const char* url) {
    const char* dot = strrchr(url, '.');
    if (!dot || dot == url) return NULL;
    return dot + 1;
}

static int is_valid_image(const char* path) {
    magic_t magic = magic_open(MAGIC_MIME_TYPE);
    if (!magic) return 0;
    magic_load(magic, NULL);
    const char* type = magic_file(magic, path);
    int valid = (type && (
        strcmp(type, "image/png") == 0 ||
        strcmp(type, "image/jpeg") == 0 ||
        strcmp(type, "image/gif") == 0));
    magic_close(magic);
    return valid;
}

bool ReplaceLinks(const char* buffer, char* out_path)
{
    regex_t regex;
    regmatch_t match[2];

    // Basic URL matcher
    const char* pattern = "(https?://[^\"]+)";
    if (regcomp(&regex, pattern, REG_EXTENDED)) return false;

    char* new_script = malloc(strlen(buffer) * 2); // generous allocation
    new_script[0] = '\0';

    const char* cursor = buffer;
    bool matched = false;
    while (regexec(&regex, cursor, 1, match, 0) == 0) {
        matched = true;
        strncat(new_script, cursor, match[0].rm_so);

        char url[MAX_URL_LEN];
        int len = match[0].rm_eo - match[0].rm_so;
        strncpy(url, cursor + match[0].rm_so, len);
        url[len] = '\0';

        const char* ext = get_extension(url);
        if (!ext || !is_supported_image_ext(ext)) {
            strcat(new_script, url);  // leave untouched
            goto next;
        }

        char filename[512];
        snprintf(filename, sizeof(filename), ".artc/%08x.%s", (unsigned)hash(url, strlen(url)), ext);

        // Download if not already downloaded
        if (access(filename, F_OK) != 0) {
            if (download_file(url, filename) != 0 || !is_valid_image(filename)) {
                fprintf(stderr, "Invalid image or download failed: %s\n", url);
                strcat(new_script, url);
                goto next;
            }
        }

        strcat(new_script, filename);

    next:
        cursor += match[0].rm_eo;
    }

    strcat(new_script, cursor);
    regfree(&regex);

    // Write to temp file
    strcpy(out_path, ".artc/tmp.lua");
    FILE* f = fopen(out_path, "w");
    if (!f) return false;
    fputs(new_script, f);
    fclose(f);

    free(new_script);
    return matched;
}

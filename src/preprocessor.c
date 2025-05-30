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
#define PATH_MAX 64

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
    regmatch_t match[1];
    const char* pattern = "(https?://[^\"]+)";
    if (regcomp(&regex, pattern, REG_EXTENDED)) return false;

    size_t buffer_len = strlen(buffer);
    size_t max_len = buffer_len * 2 + 1;
    char* new_script = malloc(max_len);
    if (!new_script) return false;
    new_script[0] = '\0';

    const char* cursor = buffer;
    bool changed = false;

    while (regexec(&regex, cursor, 1, match, 0) == 0) {
        int prefix_len = match[0].rm_so;
        strncat(new_script, cursor, prefix_len);

        const char* match_start = cursor + match[0].rm_so;
        const char* match_end   = cursor + match[0].rm_eo;

        // Extract matched URL
        int len = match_end - match_start;
        if (len >= MAX_URL_LEN) len = MAX_URL_LEN - 1;

        char url[MAX_URL_LEN];
        memcpy(url, match_start, len);
        url[len] = '\0';

        // Detect comment state up to match_start
        bool in_comment = false;
        const char* scan = buffer;
        while (scan < match_start) {
            // Single-line comment
            if (scan[0] == '-' && scan[1] == '-') {
                const char* line_end = strchr(scan, '\n');
                if (!line_end) break;
                if (match_start < line_end) {
                    in_comment = true;
                    break;
                }
                scan = line_end;
                continue;
            }

            // Multiline comment
            if (strncmp(scan, "--[[", 4) == 0) {
                const char* end = strstr(scan + 4, "]]");
                if (!end || end > match_start) {
                    in_comment = true;
                    break;
                }
                scan = end + 2;
                continue;
            }

            scan++;
        }

        if (in_comment) {
            strncat(new_script, url, max_len - strlen(new_script) - 1);
            cursor = match_end;
            continue;
        }

        // Check if it's an image link
        const char* ext = get_extension(url);
        if (!ext || !is_supported_image_ext(ext)) {
            strncat(new_script, url, max_len - strlen(new_script) - 1);
            cursor = match_end;
            continue;
        }

        // Build local filename
        char filename[512];
        snprintf(filename, sizeof(filename), ".artc/%08x.%s",
                 (unsigned)hash(url, strlen(url)), ext);

        // Download if needed
        if (access(filename, F_OK) != 0) {
            if (download_file(url, filename) != 0 || !is_valid_image(filename)) {
                fprintf(stderr, "Invalid image or download failed: %s\n", url);
                strncat(new_script, url, max_len - strlen(new_script) - 1);
                cursor = match_end;
                continue;
            }
        }

        strncat(new_script, filename, max_len - strlen(new_script) - 1);
        changed = true;
        cursor = match_end;
    }

    strncat(new_script, cursor, max_len - strlen(new_script) - 1);
    regfree(&regex);

    strncpy(out_path, ".artc/tmp.lua", PATH_MAX - 1);
    out_path[PATH_MAX - 1] = '\0';

    FILE* f = fopen(out_path, "w");
    if (!f) {
        free(new_script);
        return false;
    }

    fputs(new_script, f);
    fclose(f);
    free(new_script);

    return changed;
}

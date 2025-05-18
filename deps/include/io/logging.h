#ifndef IO_LOGGING_H
#define IO_LOGGING_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>


#define LOGGING_VERSION_MAJOR 0
#define LOGGING_VERSION_MINOR 0
#define LOGGING_VERSION_PATCH 1
#define LOGGING_VERSION "0.0.1"

#ifndef LOGAPI
    #define LOGAPI extern
#endif // LOGAPI

#define HANDLE_ERROR(msg) \
    do { \
        perror(CONCAT("[ERRO] ", msg)); \
        printf("\n"); \
        exit(1); \
    } while (0)

typedef enum {
    LOG_LEVEL_PANIC,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
} log_level_t;

LOGAPI void logging_log(log_level_t log_level, char* format, ...);

#define LOG(stream, type, format, ...) \
    do { \
        fprintf(stream, "[%s] ", type); \
        fprintf(stream, format, ##__VA_ARGS__); \
        fprintf(stream, "\n"); \
    } while(0)

#define INFO(format, ...) \
    LOG(stdout, "INFO", format, ##__VA_ARGS__)

#define ERRO(format, ...) \
    LOG(stderr, "ERRO", format, ##__VA_ARGS__)

#define WARN(format, ...) \
    LOG(stderr, "WARN", format, ##__VA_ARGS__)

#ifdef DEBUG
    #define DEBU(format, ...) \
        LOG(stderr, "DEBU", format, ##__VA_ARGS__)
#else
    #define DEBU(format, ...) 
#endif // DEBUG

#define PANIC(format, ...)                            \
    do {                                              \
        LOG(stderr, "PANIC", format, ##__VA_ARGS__);  \
        exit(1);                                      \
    } while(0)

#define DEMO(expr)                    \
    do {                              \
        LOG(stdout, "DEMO", #expr);   \
        expr;                         \
    } while(0)

#define ASSERT(expr, fmt, ...) \
    do { \
        if (!(expr)) { \
            LOG(stderr, "ERRO", "Assertion failed for (%s): " fmt, #expr, ##__VA_ARGS__); \
            exit(1); \
        } \
    } while (0)

#ifdef LOGGING_IMPLEMENTATION

LOGAPI void logging_log(log_level_t log_level, char* format, ...)
{
#ifdef LOG_LEVEL
    if(LOG_LEVEL < log_level) return;
#endif // LOG_LEVEL

    switch(log_level){
    case LOG_LEVEL_INFO:
        fprintf(stderr, "[INFO] ");
        break;
    case LOG_LEVEL_WARNING:
        fprintf(stderr, "[WARN] ");
        break;
    case LOG_LEVEL_ERROR:
        fprintf(stderr, "[ERRO] ");
        break;
    case LOG_LEVEL_DEBUG:
        fprintf(stderr, "[DEBU] ");
        break;
    case LOG_LEVEL_PANIC:
        fprintf(stderr, "[PANIC] ");
        break;
    default:
        assert(0 && "unreachable");
    }

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");

    if(log_level == LOG_LEVEL_PANIC) exit(1);
}

#endif // LOGGING_IMPLEMENTATION

#endif // IO_LOGGING_H


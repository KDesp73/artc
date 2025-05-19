#ifndef CLI_H
#define CLI_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <errno.h>

#define CLI_VERSION_MAJOR 0
#define CLI_VERSION_MINOR 0
#define CLI_VERSION_PATCH 1
#define CLI_VERSION "0.0.1"

#ifndef CLIAPI
    #define CLIAPI extern
#endif // CLIAPI

typedef struct {
    char* help;
    char* full;
    char abr;
    size_t argument_required;
} cli_arg_t;

typedef struct {
    cli_arg_t** args;
    size_t count;
    size_t capacity;
} cli_args_t;

#define LOOP_ARGS(opt, args) \
    char fmt[128]; \
    cli_generate_format_string(fmt, args); \
    struct option opts[args.count]; \
    cli_args_to_options(opts, args); \
    while((opt = getopt_long(argc, argv, fmt, opts, NULL)) != -1)

CLIAPI char* cli_shift_args(int *argc, char ***argv);

CLIAPI cli_arg_t* cli_arg_new(char abr, const char* full, const char* help, size_t argument_required);

CLIAPI cli_args_t cli_args_make(cli_arg_t* first, ...);
CLIAPI void cli_args_free(cli_args_t* arguments);

CLIAPI void cli_args_to_options(struct option options[], cli_args_t args);
CLIAPI void cli_generate_format_string(char* buffer, cli_args_t args);

CLIAPI void cli_help(cli_args_t args, const char* usage, const char* footer);

#ifdef CLI_IMPLEMENTATION
CLIAPI char* cli_shift_args(int *argc, char ***argv) {
    assert(*argc > 0);
    char *result = **argv;
    *argc -= 1;
    *argv += 1;
    return result;
}

CLIAPI cli_arg_t* cli_arg_new(char abr, const char* full, const char* help, size_t argument_required)
{
    cli_arg_t* arg = (cli_arg_t*) malloc(sizeof(cli_arg_t));

    if(full){
        arg->full = (char*) malloc(strlen(full) + 1);
        if (!arg->full) {
            free(arg);
            return NULL;
        }
        strcpy(arg->full, full);
    }

    arg->help = (char*) malloc(strlen(help) + 1);
    if (!arg->help) {
        free(arg->full);
        free(arg);
        return NULL;
    }
    strcpy(arg->help, help);

    arg->abr = abr;
    arg->argument_required = argument_required;

    return arg;
}

CLIAPI void cli_args_free(cli_args_t* arguments)
{
    for(size_t i = 0; i < arguments->count; ++i){
        free(arguments->args[i]->full);
        free(arguments->args[i]->help);
        free(arguments->args[i]);
    }
    free(arguments->args);
}

CLIAPI cli_args_t cli_args_make(cli_arg_t* first, ...)
{
    cli_args_t arguments = {.capacity = 0};

    va_list args;
    va_start(args, first);
    arguments.capacity++;

    while(va_arg(args, cli_arg_t*) != NULL) 
        arguments.capacity++;

    va_end(args);

    arguments.args = (cli_arg_t**) malloc(sizeof(arguments.args[0]) * arguments.capacity);
    
    if (arguments.args == NULL) {
        fprintf(stderr, "Could not allocate memory: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(first == NULL) return arguments;

    arguments.args[arguments.count++] = first;

    va_start(args, first);
    for (cli_arg_t* next = va_arg(args, cli_arg_t*); next != NULL; next = va_arg(args, cli_arg_t*)) {
        if(arguments.capacity == arguments.count) break;
        arguments.args[arguments.count++] = next;
    }
    va_end(args);

    return arguments;
}

CLIAPI void cli_args_to_options(struct option options[], cli_args_t args)
{
    if (args.count == 0) return;

    for (size_t i = 0; i < args.count; ++i) {
        cli_arg_t* arg = args.args[i];
        options[i].val = arg->abr;
        if(arg->full) options[i].name = arg->full;
        options[i].flag = NULL;
        options[i].has_arg = arg->argument_required;
    }
}

CLIAPI void cli_help(cli_args_t args, const char* usage, const char* footer)
{
    const char* RED = "\e[31m";
    const char* GREEN = "\e[32m";
    const char* YELLOW = "\e[33m";
    const char* RESET = "\e[0;39m";
    const char* BOLD = "\e[1m";

    int max_length = 0;

    for(size_t i = 0; i < args.count; ++i){
        if(args.args[i] == NULL) continue;

        size_t current_len = 0;
        if(args.args[i]->full == NULL)
            current_len = snprintf(NULL, 0, "-%c", args.args[i]->abr);
        else
            current_len = snprintf(NULL, 0, "-%c --%s", args.args[i]->abr, args.args[i]->full);
        if(current_len > max_length) max_length = current_len;
    }
    max_length += 2;

    printf("%sUSAGE%s\n", BOLD, RESET);
    printf("  %s\n\n", usage);

    printf("%sOPTIONS%s\n", BOLD, RESET);
    for(size_t i = 0; i < args.count; i++){
        cli_arg_t arg = *args.args[i];

        char opt[64];
        sprintf(opt, "-%c --%s", arg.abr, arg.full);

        printf("  %*s", -max_length, opt);
        printf("%s ", arg.help);

        switch(args.args[i]->argument_required){
            case no_argument:
                printf("%s[no argument]%s", RED, RESET);
                break;
            case required_argument:
                printf("%s[requires argument]%s", GREEN, RESET);
                break;
            case optional_argument:
                printf("%s[optional argument]%s", YELLOW, RESET);
                break;
        }
        printf("\n");
    }
    printf("\n%s\n", footer);
}

CLIAPI void cli_generate_format_string(char* buffer, cli_args_t args)
{
    size_t length = 1;
    for (size_t i = 0; i < args.count; ++i) {
        length += 1;
        if (args.args[i]->argument_required) {
            length += 1;
        }
    }

    buffer[0] = '\0';

    for (size_t i = 0; i < args.count; ++i) {
        char abr[2] = {args.args[i]->abr, 0};
        if(args.args[i]->argument_required == optional_argument) strcat(buffer, ":");
        strcat(buffer, abr);
        if (args.args[i]->argument_required) strcat(buffer, ":");
    }
    strcat(buffer, "\0");
}

#endif // CLI_IMPLEMENTATION

#endif // CLI_H


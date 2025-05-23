#ifndef ARTC_CLI_H
#define ARTC_CLI_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    bool export;
    char* format;
    char* output;
    bool ascii;
    bool sandbox;
    size_t durations_s;
} CliValues;

void CliValuesInit(CliValues* v);
void CliParse(CliValues* v, int argc, char** argv);

#endif // ARTC_CLI_H

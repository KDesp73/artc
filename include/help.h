#ifndef HELP_H
#define HELP_H

#include "io/ansi.h"
#include <stdio.h>

#define PI(fmt, ...) \
    printf("  " fmt "\n", ##__VA_ARGS__)

#define PB(fmt, ...) \
    do{ \
        printf("%s", ANSI_BOLD); \
        printf(fmt, ##__VA_ARGS__); \
        printf("%s\n", ANSI_RESET); \
    } while(0)

// NOTE: currently unused since the interface is simple enough to be handled by the cli.h library
static inline void help()
{
    PB("USAGE");
    PI("artc <FILE> [<OPTIONS>...]");
    printf("\n");

    PB("OPTIONS");
    PI("-h --help            Prints this message");
    PI("-v --version         Prints the current version and exits");
    PI("-x --export          Export the render in a media file");
    PI("-F --format <FMT>    Specify the format of the exported file");
    PI("-o --output <PATH>   Specify a different output path");
    PI("-A --ascii           Render visuals in the terminal");
    PI("-S --no-sandbox      Do not sandbox lua (Be careful)");
    PI("-d --duration <sec>  Exit after <sec> seconds");
    PI("-E --max-entities    Specify the max amount of entities");
    printf("\n");
    
    printf("%sWritten by KDesp73%s\n", ANSI_ITALIC, ANSI_RESET);
}


#endif // HELP_H


#ifndef IO_ANSI_H
#define IO_ANSI_H

#include <stdarg.h>


#define ANSI_VERSION_MAJOR 0
#define ANSI_VERSION_MINOR 0
#define ANSI_VERSION_PATCH 1
#define ANSI_VERSION "0.0.1"


#ifndef ANSIAPI
    #define ANSIAPI extern
#endif // ANSIAPI

#define ANSI_RESET "\e[0;39m"
#define ANSI_BOLD "\e[1m"
#define ANSI_UNDERLINE "\e[4m"
#define ANSI_ITALIC "\e[3m"
#define ANSI_CLEAR "\e[2J"
#define ANSI_ERASE_LINE "\e[2K"
#define ANSI_HIDE_CURSOR() printf("\e[?25l")
#define ANSI_SHOW_CURSOR() printf("\e[?25h")
#define ANSI_GOTOXY(x,y) printf("\e[%d;%dH", (y), (x))
#define ANSI_MOVE_CURSOR_UP(x) printf("\e[%zuA", x)
#define ANSI_MOVE_CURSOR_DOWN(x) printf("\e[%dB", x)
#define ANSI_MOVE_CURSOR_RIGHT(x) printf("\e[%dC", x)
#define ANSI_MOVE_CURSOR_LEFT(x) printf("\e[%dD", x)
#define ANSI_CLEAR_BELOW_CURSOR() printf("\e[J")
#define ANSI_CURSOR_BLOCK() printf("\033[1 q")
#define ANSI_CURSOR_UNDERSCORE() printf("\033[4 q")
#define ANSI_CURSOR_BAR() printf("\033[5 q")

#define ANSI_FG 0
#define ANSI_BG 1

#define ANSI_BLACK "\e[30m"
#define ANSI_RED "\e[31m"
#define ANSI_GREEN "\e[32m"
#define ANSI_YELLOW "\e[33m"
#define ANSI_BLUE "\e[34m"
#define ANSI_PURPLE "\e[35m"
#define ANSI_CYAN "\e[36m"
#define ANSI_LGREY "\e[37m"
#define ANSI_DGREY "\e[38m"

#define ANSI_COLOR_BG(buffer, c) ansi_color(buffer, c, 1)
#define ANSI_COLOR_FG(buffer, c) ansi_color(buffer, c, 0)

#define ANSI_WRAP(buffer, seq, fmt, ...) \
    sprintf(buffer, "%s" fmt "%s", seq, ##__VA_ARGS__, ANSI_RESET)

#define ANSI_WRAP_AND_PRINT(seq, fmt, ...) \
    printf("%s" fmt "%s", seq, ##__VA_ARGS__, ANSI_RESET)

ANSIAPI void ansi_color(char* buffer, int color, int bg);
ANSIAPI void ansi_combine(char* buffer, const char* seq1, const char* seq2);
ANSIAPI void ansi_clear_screen();
ANSIAPI void ansi_print_color_table();

#ifdef ANSI_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ANSIAPI void ansi_combine(char* buffer, const char* seq1, const char* seq2)
{
    if (seq1 == NULL || seq2 == NULL) {
        return;
    }

    const char* prefix = "\e[";
    const char* suffix = "m";

    const char* seq1_inner = seq1 + 2; // Skip \e[
    size_t seq1_len = strlen(seq1_inner) - 1; // Remove the trailing 'm'

    const char* seq2_inner = seq2 + 2; // Skip \e[
    size_t seq2_len = strlen(seq2_inner) - 1; // Remove the trailing 'm'

    sprintf(buffer, "%s%.*s;%.*s%s", prefix, (int)seq1_len, seq1_inner, (int)seq2_len, seq2_inner, suffix);
}

ANSIAPI void ansi_color(char* buffer, int color, int bg)
{
    if (color < 0 || color > 255) return;
    sprintf(buffer, "\e[%d8;5;%dm", bg+3, color);
}

ANSIAPI void ansi_clear_screen()
{
    printf("\e[2J\e[H");
}

ANSIAPI void ansi_print_color_table()
{
    for(int i = 0; i < 256; i++){
        if(i % 21 == 0) printf("\n");
        
        char color[128];
        ansi_color(color, i, 0);
        printf("%s%3d ", color, i);
    }
    printf("%s\n", ANSI_RESET);
}

#endif // ANSI_IMPLEMENTATION

#endif // IO_ANSI_H


#ifndef PALETTE_H
#define PALETTE_H

typedef struct {
    char* white;
    char* black;
    char* red;
    char* green;
    char* blue;
    char* orange;
    char* purple;
    char* yellow;
    char* pink;
    char* grey;
    char* light_blue;
    char* light_grey;
} Palette;

#define CREATE_PALETTE(name, ...) \
    Palette Palette##name() { \
        return (Palette){ __VA_ARGS__ }; \
    }

CREATE_PALETTE(CatppuccinMacchiato,
    .red = "#ed8796",
    .yellow = "#eed49f",
    .pink = "#f5bde6",
    .purple = "#c6a0f6",
    .blue = "#8aadf4",
    .grey = "#24273a",
    .black = "#181926",
    .green = "#a6da95",
    .white = "#cad3f5",
    .orange = "#f5a97f",
    .light_blue = "#91d7e3",
    .light_grey = "#8087a2",
)

// TODO: add more...

#endif // PALETTE_H

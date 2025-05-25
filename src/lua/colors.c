#include "lua/lauxlib.h"
#include "lua/lua.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

static int clamp(int v)
{
    if (v < 0) return 0;
    if (v > 255) return 255;
    return v;
}

int lua_rgb_object_to_hex(lua_State* L)
{
    if (!lua_istable(L, 1)) {
        return luaL_error(L, "Expected a table as argument");
    }

    lua_getfield(L, 1, "r");
    int r = (int)luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "g");
    int g = (int)luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "b");
    int b = (int)luaL_checknumber(L, -1);
    lua_pop(L, 1);

    r = clamp(r);
    g = clamp(g);
    b = clamp(b);

    char hex[8];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X", r, g, b);

    lua_pushstring(L, hex);
    return 1;
}


// Helper: Convert RGB components (0-255) to hex string "#RRGGBB"
static void rgb_to_hex(int r, int g, int b, char* out_hex)
{
    snprintf(out_hex, 8, "#%02X%02X%02X", clamp(r), clamp(g), clamp(b));
}

// Helper: Parse hex string "#RRGGBB" to RGB components (0-255)
static int hex_to_rgb(const char* hex, int* r, int* g, int* b)
{
    if (hex[0] != '#' || strlen(hex) != 7) return 0;
    unsigned int ri, gi, bi;
    if (sscanf(hex+1, "%02x%02x%02x", &ri, &gi, &bi) != 3) return 0;
    *r = (int)ri;
    *g = (int)gi;
    *b = (int)bi;
    return 1;
}

// Helper: Convert RGB (0-255) to HSL (h:0-360, s:0-1, l:0-1)
static void rgb_to_hsl(int r, int g, int b, double* h, double* s, double* l)
{
    double rr = r / 255.0;
    double gg = g / 255.0;
    double bb = b / 255.0;

    double max = fmax(rr, fmax(gg, bb));
    double min = fmin(rr, fmin(gg, bb));
    *l = (max + min) / 2.0;

    if (max == min) {
        *h = 0;
        *s = 0;
    } else {
        double d = max - min;
        *s = (*l > 0.5) ? d / (2.0 - max - min) : d / (max + min);

        if (max == rr) {
            *h = (gg - bb) / d + (gg < bb ? 6 : 0);
        } else if (max == gg) {
            *h = (bb - rr) / d + 2;
        } else {
            *h = (rr - gg) / d + 4;
        }
        *h *= 60.0;
    }
}

// Helper: Parse RGB table from Lua stack (either table or 3 numbers)
static int lua_parse_rgb(lua_State* L, int index, int* r, int* g, int* b)
{
    if (lua_istable(L, index)) {
        lua_getfield(L, index, "r");
        *r = (int)luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, index, "g");
        *g = (int)luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, index, "b");
        *b = (int)luaL_checknumber(L, -1);
        lua_pop(L, 1);
        return 1;
    } else if (lua_isnumber(L, index) && lua_isnumber(L, index+1) && lua_isnumber(L, index+2)) {
        *r = (int)luaL_checknumber(L, index);
        *g = (int)luaL_checknumber(L, index+1);
        *b = (int)luaL_checknumber(L, index+2);
        return 3; // number of args consumed
    } else {
        luaL_error(L, "Expected rgb({r,g,b}) or rgb(r,g,b)");
        return 0;
    }
}

// color.rgb(...) Lua function: rgb({r,g,b}) or rgb(r,g,b) → hex string
static int lua_rgb(lua_State* L)
{
    int r, g, b;
    int nargs = lua_gettop(L);
    if (nargs == 1) {
        lua_parse_rgb(L, 1, &r, &g, &b);
    } else if (nargs == 3) {
        r = (int)luaL_checknumber(L, 1);
        g = (int)luaL_checknumber(L, 2);
        b = (int)luaL_checknumber(L, 3);
    } else {
        return luaL_error(L, "rgb() expects either a table or 3 numbers");
    }
    r = clamp(r);
    g = clamp(g);
    b = clamp(b);
    char hex[8];
    rgb_to_hex(r, g, b, hex);
    lua_pushstring(L, hex);
    return 1;
}

// color.to_rgb("#RRGGBB") → {r=..., g=..., b=...}

static int lua_to_rgb(lua_State* L)
{
    const char* hex = luaL_checkstring(L, 1);
    int r, g, b;
    if (!hex_to_rgb(hex, &r, &g, &b)) {
        return luaL_error(L, "Invalid hex color string");
    }
    lua_newtable(L);
    lua_pushinteger(L, r);
    lua_setfield(L, -2, "r");
    lua_pushinteger(L, g);
    lua_setfield(L, -2, "g");
    lua_pushinteger(L, b);
    lua_setfield(L, -2, "b");
    return 1;
}

static float hue_to_rgb(float p, float q, float t) {
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;
    if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f / 2.0f) return q;
    if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
    return p;
}

static int lua_hsl(lua_State* L)
{
    float h = luaL_checknumber(L, 1);
    float s = luaL_checknumber(L, 2);
    float l = luaL_checknumber(L, 3);

    h = fmodf(h, 360.0f) / 360.0f; // Normalize hue to [0,1]
    s = s / 100.0f; // Scale saturation to [0,1]
    l = l / 100.0f; // Scale lightness to [0,1]

    float r, g, b;

    if (s == 0.0f) {
        r = g = b = l; // Achromatic
    } else {
        float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        float p = 2.0f * l - q;
        r = hue_to_rgb(p, q, h + 1.0f / 3.0f);
        g = hue_to_rgb(p, q, h);
        b = hue_to_rgb(p, q, h - 1.0f / 3.0f);
    }

    int ri = clamp((int)(r * 255.0f));
    int gi = clamp((int)(g * 255.0f));
    int bi = clamp((int)(b * 255.0f));

    char hex[8];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X", ri, gi, bi);
    lua_pushstring(L, hex);
    return 1;
}

// color.to_hsl("#RRGGBB") → {h=..., s=..., l=...}
static int lua_to_hsl(lua_State* L)
{
    const char* hex = luaL_checkstring(L, 1);
    int r, g, b;
    if (!hex_to_rgb(hex, &r, &g, &b)) {
        return luaL_error(L, "Invalid hex color string");
    }
    double h, s, l;
    rgb_to_hsl(r, g, b, &h, &s, &l);

    lua_newtable(L);
    lua_pushnumber(L, h);
    lua_setfield(L, -2, "h");
    lua_pushnumber(L, s);
    lua_setfield(L, -2, "s");
    lua_pushnumber(L, l);
    lua_setfield(L, -2, "l");
    return 1;
}

void register_color_module(lua_State* L)
{
    static const struct luaL_Reg colorlib[] = {
        {"rgb", lua_rgb},
        {"to_rgb", lua_to_rgb},
        {"hsl", lua_hsl},
        {"to_hsl", lua_to_hsl},
        {NULL, NULL}
    };

    luaL_newlib(L, colorlib);
    lua_setglobal(L, "color");
}


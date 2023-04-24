#pragma once

// Utility functions for rendering

#include "common.h"

#include "colormap.h"

#include "SDL/SDL_ttf.h"
#include "SDL/SDL_video.h"

// Temporary function, to quickly get something visible on screen
// should be replaced by a proper solution
void r_draw_text(SDL_Surface* target, const char* str, i32 x, i32 y, TTF_Font* font, cm_color color);

void r_draw_pixel(SDL_Surface* target, i32 x, i32 y, cm_color color);
void r_draw_line(SDL_Surface* target, i32 x_a, i32 y_a, i32 x_b, i32 y_b, cm_color color);

static bool r_clip_line(i32 x_max, i32 y_max, i32* x_a, i32* y_a, i32* x_b, i32* y_b);
static inline u8 r_clip_code(i32 x_max, i32 y_max, i32 x, i32 y);
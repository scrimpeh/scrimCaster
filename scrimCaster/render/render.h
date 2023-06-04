#pragma once

// This module handles everything related to rendering

// In 3D space, we use a basic raycaster with cosine correction, as you would expect
// In 2D space, to be independent of pixel sizes, sizes are treated as floats in [0., 1.[
// with the value denoting how far we are from the edge of the screen

#include <common.h>

#include <geometry.h>
#include <map/cell.h>
#include <render/color/colormap.h>

#include <SDL/SDL_video.h>

#define R_CELL_H M_CELLHEIGHT
#define R_HALF_H (R_CELL_H / 2)

extern const cm_color COLOR_KEY;

i32 r_init(u16 w, u16 h);

void r_close();
void r_draw();

i16 r_hud_px_h(float x);
i16 r_hud_px_v(float y);
float r_hud_hu_h(i16 x);
float r_hud_hu_v(i16 y);

static void r_draw_crosshair(SDL_Surface* target);

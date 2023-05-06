#pragma once

#include <common.h>

#include <render/color/colormap.h>

#define ATAN_INTERNAL(a, halfcircle, circle) ((a) < (halfcircle) ? (a) : (-(circle)) + (a))
#define ATAN_RAD(a) ATAN_INTERNAL(a, M_PI, M_PI * 2)
#define ATAN_DEG(a) ATAN_INTERNAL(a, 180, 360)

// TODO: This is also defined in texture.h
#define TEX_SIZE 64
#define TEX_MAP_SIZE 1024

#define R_CELL_H M_CELLHEIGHT
#define R_HALF_H (R_CELL_H / 2)

extern const cm_color COLOR_KEY;

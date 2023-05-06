#pragma once

#include <common.h>

#include <map/cell.h>
#include <render/color/colormap.h>

// Light that is smoothly blended between cells.

// Currently, there are three different smooth lighting modes:
//
// 1. R_LIGHT_SMOOTH_CORNER
// Light is sampled at the four corners of every cell. Within a cell, light is bilinearly interpolated
//
// 2. R_LIGHT_SMOOTH_EDGE
// Light is sampled at the edge of each cell and averaged between the x and y direction. (This doesn't really work as I had hoped)
//
// 3. R_LIGHT_SMOOTH_FINE
// Like R_LIGHT_SMOOTH_CORNER, but the density of the sampling grid is doubled. 

i32 r_light_smooth_init();
void r_light_smooth_destroy();

typedef struct
{
	u8 brightness[3][3];
} r_light_smooth_cell_brightness;

static u8 r_light_smooth_brightness_side(i32 x, i32 y, m_orientation orientation);
static u8 r_light_smooth_brightness_corner(i32 x, i32 y, m_orientation next);
static bool r_light_smooth_block(i32 x, i32 y, m_orientation orientation);
static bool r_light_smooth_side_block(const m_side* side);

u8 r_light_smooth_corner_get(i32 map_x, i32 map_y, m_orientation orientation, u8 x, u8 y);
u8 r_light_smooth_edge_get(i32 map_x, i32 map_y, m_orientation orientation, u8 x, u8 y);
u8 r_light_smooth_fine_get(i32 map_x, i32 map_y, m_orientation orientation, u8 x, u8 y);

static void r_light_smooth_get_cell_xy(u8 x, u8 y, m_orientation orientation, u8* light_x, u8* light_y);

i32 r_light_smooth_update(i16 x_start, i16 y_start, i16 x_end, i16 y_end);
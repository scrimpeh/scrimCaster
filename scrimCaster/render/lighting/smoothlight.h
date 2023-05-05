#pragma once

#include <common.h>

#include <map/cell.h>
#include <render/colormap.h>

// Light that is smoothly blended between cells. For every cell, light is sampled at the four corner points first
// A light pixel is then smoothly blended between the four corner points.
// Ordinarily, adjacent corners have the same brightness value, unless there is a solid or smooth-light blocking wall between
// them, in which case the hard edge remains.

i32 r_light_smooth_init();
void r_light_smooth_destroy();

typedef struct
{
	u8 ne;
	u8 nw;
	u8 sw;
	u8 se;
} r_light_smooth_cell_brightness;

static u8 r_light_smooth_brightness_2(const m_cell* curr, const m_cell* other, m_orientation orientation);
static u8 r_light_smooth_brightness_4(const m_cell* a, const m_cell* b, const m_cell* c, const m_cell* d, m_orientation next);
 
static bool r_light_smooth_block(const m_side* side);

u8 r_light_smooth_get_brightness(i32 map_x, i32 map_y, m_orientation orientation, u8 x, u8 y);
#pragma once

#include <common.h>

#include <map/cell.h>
#include <render/colormap.h>

typedef enum
{
	R_LIGHT_FULLBRIGHT,
	R_LIGHT_FLAT,
	R_LIGHT_SMOOTH
} r_light_type;

extern r_light_type r_light;

cm_color r_light_px(i32 map_x, i32 map_y, m_orientation orientation, cm_color px, u8 x, u8 y);
#pragma once

#include <common.h>

#include <map/cell.h>
#include <render/colormap.h>

typedef enum
{
	R_LIGHT_NONE,
	R_LIGHT_SHADED,
	R_LIGHT_FLAT,
	R_LIGHT_SMOOTH
} r_light_type;

extern r_light_type r_light;

i32 r_light_init(r_light_type light_type);
void r_light_destroy();

cm_color r_light_px(i32 map_x, i32 map_y, m_orientation orientation, cm_color px, u8 x, u8 y);
#pragma once

#include <common.h>

#include <map/cell.h>
#include <render/color/colormap.h>

typedef enum
{
	R_LIGHT_NONE,
	R_LIGHT_SHADED,
	R_LIGHT_FLAT,
	R_LIGHT_SMOOTH_CORNER,
	R_LIGHT_SMOOTH_EDGE,
	R_LIGHT_SMOOTH_FINE
} r_light_type;

extern r_light_type r_light;

i32 r_light_init(r_light_type light_type);
void r_light_destroy();

i32 r_light_update(i16 x_start, i16 y_start, i16 x_end, i16 y_end);

cm_color r_light_px(i32 map_x, i32 map_y, m_orientation orientation, cm_color px, u8 x, u8 y);
float r_light_get_alpha(i32 map_x, i32 map_y, m_orientation orientation, u8 x, u8 y);

cm_color r_light_apply(cm_color px, float alpha);
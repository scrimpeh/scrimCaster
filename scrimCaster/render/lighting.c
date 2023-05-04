#include <render/lighting.h>

#include <map/map.h>
#include <util/mathutil.h>

// In theory, we could even go as far as defining lightmaps for the floors and walls, but that'd go too far

r_light_type r_light = R_LIGHT_FLAT;
r_light_type r_light_side_shade = 48;

cm_color r_light_px(i32 map_x, i32 map_y, m_orientation orientation, cm_color px, u8 x, u8 y)
{
	if (r_light == R_LIGHT_FLAT)
	{
		const u8 cell_brightness = m_get_cell(map_x, map_y)->brightness;
		float brightness = cell_brightness;
		if (orientation == M_EAST || orientation == M_WEST)
			brightness = SDL_max(brightness - r_light_side_shade, 0);
		const float alpha = brightness / ((1 << sizeof(cell_brightness) * 8) - 1);
		return cm_map(px, CM_GET(0, 0, 0), 1.0f - alpha);
	}
	else if (r_light == R_LIGHT_SMOOTH)
	{
		// Smooth lighting
		return px;
	}
	else
	{
		// Fullbright
		return px;
	}
}
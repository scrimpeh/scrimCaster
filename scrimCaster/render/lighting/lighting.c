#include <render/lighting/lighting.h>

#include <map/map.h>
#include <util/mathutil.h>
#include <render/lighting/smoothlight.h>

// In theory, we could even go as far as defining lightmaps for the floors and walls, but that'd go too far

r_light_type r_light = R_LIGHT_NONE;

u8 r_light_side_shade[] =
{
	[R_LIGHT_NONE] =    0,
	[R_LIGHT_SHADED] = 48,
	[R_LIGHT_FLAT] =   48,
	[R_LIGHT_SMOOTH] = 16
};

#define R_FULLBRIGHT ((1 << (sizeof(u8) * 8)) - 1)

i32 r_light_init(r_light_type light_type)
{
	r_light_destroy();
	r_light = light_type;
	switch (r_light)
	{
	case R_LIGHT_SMOOTH:
		return r_light_smooth_init();
	default:
		// Nothing to do
		return 0;
	}
}

void r_light_destroy()
{
	switch (r_light)
	{
	case R_LIGHT_SMOOTH:
		r_light_smooth_destroy();
		break;
	}
	r_light = R_LIGHT_NONE;
}

cm_color r_light_px(i32 map_x, i32 map_y, m_orientation orientation, cm_color px, u8 x, u8 y)
{
	u8 brightness = 0;
	if (r_light == R_LIGHT_SHADED)
		brightness = R_FULLBRIGHT;
	else if (r_light == R_LIGHT_FLAT)
		brightness = m_get_cell(map_x, map_y)->brightness;
	else if (r_light == R_LIGHT_SMOOTH)
		brightness = r_light_smooth_get_brightness(map_x, map_y, orientation, x, y);
	else
		return px;

	if (orientation == M_EAST || orientation == M_WEST)
		brightness = SDL_max(brightness - r_light_side_shade[r_light], 0);
	const float alpha = (float) brightness / ((1 << sizeof(u8) * 8) - 1);
	return cm_map(px, CM_GET(0, 0, 0), 1.0f - alpha);
}

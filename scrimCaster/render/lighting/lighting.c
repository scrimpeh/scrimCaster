#include <render/lighting/lighting.h>

#include <map/map.h>
#include <util/mathutil.h>
#include <render/lighting/smoothlight.h>

// In theory, we could even go as far as defining lightmaps for the floors and walls, but that'd go too far

r_light_type r_light = R_LIGHT_NONE;
u8 r_light_quantization = 0x1;

u8 r_light_side_shade[] =
{
	[R_LIGHT_NONE]          =  0,
	[R_LIGHT_SHADED]        = 48,
	[R_LIGHT_FLAT]          = 48,
	[R_LIGHT_SMOOTH_CORNER] = 16,
	[R_LIGHT_SMOOTH_EDGE]   = 16,
	[R_LIGHT_SMOOTH_FINE]   = 16
};

#define R_FULLBRIGHT ((1 << (sizeof(u8) * 8)) - 1)

i32 r_light_init(r_light_type light_type)
{
	r_light_destroy();
	r_light = light_type;
	switch (r_light)
	{
	case R_LIGHT_SMOOTH_CORNER:
	case R_LIGHT_SMOOTH_EDGE:
	case R_LIGHT_SMOOTH_FINE:
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
	case R_LIGHT_SMOOTH_CORNER:
	case R_LIGHT_SMOOTH_EDGE:
	case R_LIGHT_SMOOTH_FINE:
		r_light_smooth_destroy();
		break;
	}
	r_light = R_LIGHT_NONE;
}

i32 r_light_update(i16 x_start, i16 y_start, i16 x_end, i16 y_end)
{
	switch (r_light)
	{
	case R_LIGHT_SMOOTH_CORNER:
	case R_LIGHT_SMOOTH_EDGE:
	case R_LIGHT_SMOOTH_FINE:
		return r_light_smooth_update(x_start, y_start, x_end, y_end);
		break;
	}
	return 0;
}

float r_light_get_alpha(i32 map_x, i32 map_y, m_orientation o, u8 x, u8 y)
{
	u8 brightness = 0;
	switch (r_light)
	{
	case R_LIGHT_SHADED:        brightness = R_FULLBRIGHT;                                     break;
	case R_LIGHT_FLAT:          brightness = m_get_cell(map_x, map_y)->brightness;             break;
	case R_LIGHT_SMOOTH_CORNER: brightness = r_light_smooth_corner_get(map_x, map_y, o, x, y); break;
	case R_LIGHT_SMOOTH_EDGE:   brightness = r_light_smooth_edge_get(map_x, map_y, o, x, y);   break;
	case R_LIGHT_SMOOTH_FINE:   brightness = r_light_smooth_fine_get(map_x, map_y, o, x, y);   break;
	default:                    return R_FULLBRIGHT;
	}

	if (o == M_EAST || o == M_WEST)
		brightness = SDL_max(brightness - r_light_side_shade[r_light], 0);
	
	brightness &= ~(r_light_quantization - 1);
	const float alpha = (float) brightness / ((1 << sizeof(u8) * 8) - 1);
	return 1.0 - alpha;
}

cm_color r_light_apply(cm_color px, float alpha)
{
	return cm_map(px, CM_GET(0, 0, 0), alpha);
}
#include <render/lighting/smoothlight.h>

#include <map/map.h>
#include <util/mathutil.h>

static r_light_smooth_cell_brightness* r_cell_lights = NULL;

#define R_FINE_HALFSIZE (M_CELLSIZE / 2)

i32 r_light_smooth_init()
{
	r_cell_lights = SDL_malloc(m_map.w * m_map.h * sizeof(r_light_smooth_cell_brightness));
	if (!r_cell_lights)
		return -1;

	return r_light_smooth_update(0, 0, m_map.w, m_map.h);
}

void r_light_smooth_destroy()
{
	SDL_free(r_cell_lights);
	r_cell_lights = NULL;
}


static bool r_light_smooth_side_block(const m_side* side)
{
	return !side || (side->type && !(side->flags & TRANSLUCENT)) || (side->flags & BLOCK_SMOOTH_LIGHT);
}

static u8 r_light_smooth_brightness_side(i32 x, i32 y, m_orientation orientation)
{
	return r_light_smooth_block(x, y, orientation) ?
		m_get_cell(x, y)->brightness :
		(m_get_cell(x, y)->brightness + m_get_next_cell(x, y, orientation)->brightness) / 2;
}

static u8 r_light_smooth_brightness_corner(i32 x, i32 y, m_orientation next)
{
	u8 light = 0;
	u8 light_levels[4];
	for (u32 i = 0; i < 4; i++)
	{
		light_levels[i] = x >= 0 && x < m_map.w && y >= 0 && y < m_map.h ? m_get_cell(x, y)->brightness : 0;
		light |= r_light_smooth_block(x, y, next) << i;
		x = m_get_next_x(x, next);
		y = m_get_next_y(y, next);
		next = (next + 1) % 4;
	}

	switch (light)
	{
	case 0x0:
		// 3   2
		//  
		// 0   1
		return (light_levels[0] + light_levels[1] + light_levels[2] + light_levels[3]) / 4;
	case 0x1:
		// 3   2
		//   +
		// 0 | 1
		return (light_levels[0] + light_levels[1] + light_levels[2] + light_levels[3]) / 4;
	case 0x2:
		// 3   2
		//   +--
		// 0   1
		return (light_levels[0] + light_levels[1] + light_levels[2] + light_levels[3]) / 4;
	case 0x3:
		// 3   2
		//   +--
		// 0 | 1
		return (light_levels[0] + light_levels[2] + light_levels[3]) / 3;
	case 0x4:
		// 3 | 2
		//   +  
		// 0   1
		return (light_levels[0] + light_levels[2] + light_levels[3]) / 3;
	case 0x5:
		// 3 | 2
		//   +  
		// 0 | 1
		return (light_levels[0] + light_levels[3]) / 2;
	case 0x6:
		// 3 | 2
		//   +--
		// 0   1
		return (light_levels[0] + light_levels[1] + light_levels[3]) / 3;
	case 0x7:
		// 3 | 2
		//   +--
		// 0 | 1
		return (light_levels[0] + light_levels[3]) / 2;
	case 0x8:
		// 3   2
		// --+
		// 0   1
		return (light_levels[0] + light_levels[1] + light_levels[2] + light_levels[3]) / 4;
	case 0x9:
		// 3   2
		// --+
		// 0 | 1
		return light_levels[0];
	case 0xA:
		// 3   2
		// --+--
		// 0   1
		return (light_levels[0] + light_levels[1]) / 2;
	case 0xB:
		// 3   2
		// --+--
		// 0 | 1
		return light_levels[0];
	case 0xC:
		// 3 | 2
		// --+  
		// 0   1
		return (light_levels[0] + light_levels[1] + light_levels[2]) / 3;
	case 0xD:
		// 3 | 2
		// --+  1
		// 0 | 1
		return light_levels[0];
	case 0xE:
		// 3 | 2
		// --+--
		// 0   1
		return (light_levels[0] + light_levels[1]) / 2;
	case 0xF:
		// 3 | 2
		// --+--
		// 0 | 1
		return light_levels[0];
	}
}

static bool r_light_smooth_block(i32 x, i32 y, m_orientation orientation)
{
	if (x < 0 || x >= m_map.w || y < 0 || y >= m_map.h)
		return true;
	const m_side* side = m_get_side(x, y, orientation);
	const m_side* opposite = m_get_opposite_side(x, y, orientation);
	return r_light_smooth_side_block(side) || r_light_smooth_side_block(opposite);
}


u8 r_light_smooth_corner_get(i32 map_x, i32 map_y, m_orientation orientation, u8 x, u8 y)
{
	u8 cell_x;
	u8 cell_y;
	r_light_smooth_get_cell_xy(x, y, orientation, &cell_x, &cell_y);

	const r_light_smooth_cell_brightness* cell = &r_cell_lights[map_y * m_map.w + map_x];

	// bilinear interpolation
	const float x_a = math_lerp(0, M_CELLSIZE - 1, cell_x, cell->brightness[0][0], cell->brightness[0][2]);
	const float x_b = math_lerp(0, M_CELLSIZE - 1, cell_x, cell->brightness[2][0], cell->brightness[2][2]);
	return (u8) math_lerp(0, M_CELLSIZE - 1, cell_y, x_a, x_b);
}

u8 r_light_smooth_edge_get(i32 map_x, i32 map_y, m_orientation orientation, u8 x, u8 y)
{
	u8 cell_x;
	u8 cell_y;
	r_light_smooth_get_cell_xy(x, y, orientation, &cell_x, &cell_y);

	const r_light_smooth_cell_brightness* cell = &r_cell_lights[map_y * m_map.w + map_x];

	const float x_a = math_lerp(0, M_CELLSIZE - 1, cell_x, cell->brightness[1][0], cell->brightness[1][2]);
	const float y_a = math_lerp(0, M_CELLSIZE - 1, cell_y, cell->brightness[0][1], cell->brightness[2][1]);

	// TODO: This doesn't work as I expected
	return (x_a + y_a) / 2;
}

u8 r_light_smooth_fine_get(i32 map_x, i32 map_y, m_orientation orientation, u8 x, u8 y)
{
	u8 cell_x;
	u8 cell_y;
	r_light_smooth_get_cell_xy(x, y, orientation, &cell_x, &cell_y);

	const r_light_smooth_cell_brightness* cell = &r_cell_lights[map_y * m_map.w + map_x];

	const u8 x_fine = cell_x / R_FINE_HALFSIZE;
	const u8 y_fine = cell_y / R_FINE_HALFSIZE;

	const u8 x_fine_start = x_fine * R_FINE_HALFSIZE;
	const u8 x_fine_end = x_fine_start + R_FINE_HALFSIZE - 1;
	const u8 y_fine_start = y_fine * R_FINE_HALFSIZE;
	const u8 y_fine_end = y_fine_start + R_FINE_HALFSIZE - 1;

	const float x_a = math_lerp(x_fine_start, x_fine_end, cell_x, cell->brightness[y_fine][x_fine], cell->brightness[y_fine][x_fine + 1]);
	const float x_b = math_lerp(x_fine_start, x_fine_end, cell_x, cell->brightness[y_fine + 1][x_fine], cell->brightness[y_fine + 1][x_fine + 1]);
	return math_lerp(y_fine_start, y_fine_end, cell_y, x_a, x_b);
}

static void r_light_smooth_get_cell_xy(u8 x, u8 y, m_orientation orientation, u8* cell_x, u8* cell_y)
{
	switch (orientation)
	{
	case M_EAST:
		*cell_x = M_CELLSIZE - 1;
		*cell_y = x;
		break;
	case M_NORTH:
		*cell_x = x;
		*cell_y = 0;
		break;
	case M_WEST:
		*cell_x = 0;
		*cell_y = M_CELLSIZE - x - 1;
		break;
	case M_SOUTH:
		*cell_x = M_CELLSIZE - x - 1;
		*cell_y = M_CELLSIZE - 1;
		break;
	default:
		*cell_x = x;
		*cell_y = y;
		break;
	}
}

i32 r_light_smooth_update(i16 x_start, i16 y_start, i16 x_end, i16 y_end)
{
	for (i32 y = y_start - 1; y <= y_end - 1; y++)
	{
		for (i32 x = x_start - 1; x <= x_end + 1; x++)
		{
			if (x < 0 || x >= m_map.w || y < 0 || y >= m_map.h)
				continue;

			r_light_smooth_cell_brightness* current = &r_cell_lights[y * m_map.w + x];

			// Orientation map
			// N N E
			// W - E
			// W S S
			current->brightness[0][0] = r_light_smooth_brightness_corner(x, y, M_NORTH);
			current->brightness[0][1] = r_light_smooth_brightness_side(x, y, M_NORTH);
			current->brightness[0][2] = r_light_smooth_brightness_corner(x, y, M_EAST);
			current->brightness[1][0] = r_light_smooth_brightness_side(x, y, M_WEST);
			current->brightness[1][1] = m_get_cell(x, y)->brightness;
			current->brightness[1][2] = r_light_smooth_brightness_side(x, y, M_EAST);
			current->brightness[2][0] = r_light_smooth_brightness_corner(x, y, M_WEST);
			current->brightness[2][1] = r_light_smooth_brightness_side(x, y, M_SOUTH);
			current->brightness[2][2] = r_light_smooth_brightness_corner(x, y, M_SOUTH);
		}
	}

	return 0;
}
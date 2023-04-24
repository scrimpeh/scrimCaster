#include "geometry.h"

#include "map.h"
#include "renderconstants.h"

#include <math.h>

extern Map map;

const float MAXSLOPE = 1e+8f;

void g_cast(float x, float y, angle_rad_f angle, g_intercept_collector intercept_collector)
{
	i16 grid_x = (i16)floorf(x / CELLSIZE);
	i16 grid_y = (i16)floorf(y / CELLSIZE);

	// Since our grid inverts the y coordinate, invert the slope
	g_orientation orientation = g_get_orientation(angle);
	float slope = -1 * tanf(angle);
	float dx = (!g_is_east(orientation) ? grid_x : grid_x + 1) * CELLSIZE - x;

	if (fabsf(slope) > MAXSLOPE)
		slope = slope > 0 ? MAXSLOPE : -MAXSLOPE;
	while (true)
	{
		x += dx;
		y += dx * slope;
		const i16 next_grid_y = (i16)floorf(y / CELLSIZE);

		// Trace grid vertically first
		for (i16 y_cell = grid_y; y_cell != next_grid_y; y_cell += g_is_north(orientation) ? -1 : 1)
		{
			// Out-of-bounds check
			if (y_cell < 0 || y_cell >= map.h || grid_x < 0 || grid_x >= map.w)
				return;

			const Cell* cell = &map.cells[map.w * y_cell + grid_x];
			const Side* side = g_is_north(orientation) ? &cell->n : &cell->s;
			const bool edge =
				(g_is_north(orientation) && y_cell == 0) ||
				(g_is_south(orientation) && y_cell == map.h - 1);

			if (edge || side->type)
			{
				// We hit something - invert the sloping operation to determine the exact position of the intercept
				const i16 y_cell_side = g_is_north(orientation) ? y_cell : y_cell + 1;
				float x_inv = x - dx;
				float y_inv = y - (dx * slope);
				x_inv += ((y_cell_side * CELLSIZE) - y_inv) / slope;
				y_inv = (float)(y_cell_side * CELLSIZE);

				g_intercept intercept =
				{
					g_is_north(orientation) ? SIDE_ORIENTATION_NORTH : SIDE_ORIENTATION_SOUTH,
					g_get_intercept_type(side, edge),
					angle,
					x_inv,
					y_inv,
					grid_x,
					y_cell,
					g_is_north(orientation) ? (u32)floorf(x_inv) % TEX_SIZE : TEX_SIZE - 1 - (u32)floorf(x_inv) % TEX_SIZE
				};

				// Add the intercept
				if (!intercept_collector(&intercept))
					return;
			}
		}

		// Now look horizontally
		grid_y = next_grid_y;

		// Out-of-bounds check
		if (grid_y < 0 || grid_y >= map.h || grid_x < 0 || grid_x >= map.w)
			return;

		const Cell* cell = &map.cells[map.w * grid_y + grid_x];
		const Side* side = g_is_west(orientation) ? &cell->w : &cell->e;
		const bool edge =
			(g_is_west(orientation) && grid_x == 0) ||
			(g_is_east(orientation) && grid_x == map.w - 1);
			
		if (edge || side->type)
		{
			g_intercept intercept =
			{
				g_is_west(orientation) ? SIDE_ORIENTATION_WEST : SIDE_ORIENTATION_EAST,
				g_get_intercept_type(side, edge),
				angle,
				x,
				y,
				grid_x,
				grid_y,
				g_is_east(orientation) ? (u32)floorf(y) % TEX_SIZE : TEX_SIZE - 1 - (u32)floorf(y) % TEX_SIZE
			};
			// Add the intercept
			if (!intercept_collector(&intercept))
				return;
		}

		// Didn't find anything
		dx = g_is_east(orientation) ? float(CELLSIZE) : float(-CELLSIZE);
		grid_x += g_is_east(orientation) ? 1 : -1;
	}
}

static inline g_orientation g_get_orientation(angle_rad_f angle)
{
	g_orientation orientation = 0;
	if (angle < M_PI)
		orientation |= ORIENTATION_NORTH;
	if (angle < PI_1_2 || angle > PI_3_2)
		orientation |= ORIENTATION_EAST;
	return orientation;
}

static inline bool g_is_north(g_orientation orientation)
{
	return orientation & ORIENTATION_NORTH;
}

static inline bool g_is_east(g_orientation orientation)
{
	return orientation & ORIENTATION_EAST;
}

static inline bool g_is_west(g_orientation orientation)
{
	return !g_is_east(orientation);
}

static inline bool g_is_south(g_orientation orientation)
{
	return !g_is_north(orientation);
}


static inline g_intercept_type g_get_intercept_type(const Side* side, bool is_edge)
{
	g_intercept_type intercept_type = G_INTERCEPT_VOID;
	if (!is_edge)
		intercept_type = side->flags & TRANSLUCENT ? G_INTERCEPT_NON_SOLID : G_INTERCEPT_SOLID;
	return intercept_type;
}

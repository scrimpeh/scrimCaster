#include "geometry.h"

#include "map.h"
#include "renderconstants.h"

#include <math.h>

extern Map map;

const float MAXSLOPE = 1e+8f;

void g_cast(float x, float y, angle_rad_f angle, g_intercept_collector intercept_collector)
{
	g_orientation orientation = g_get_orientation(angle);

	i16 grid_x = (i16)floorf(x / CELLSIZE);
	i16 grid_y = (i16)floorf(y / CELLSIZE);
	// Since our grid inverts the y coordinate, invert the slope
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
			const bool y_void = y_cell < 0 || y_cell >= map.boundsY;
			const bool y_edge = 
				(g_is_north(orientation) && y_cell == 0) || 
				(g_is_south(orientation) && y_cell == map.boundsY - 1);
			bool y_void_edge = false;
			const Side* y_side = NULL;
			if (!y_void)
			{
				const Cell* cell = &map.cells[map.boundsX * y_cell + grid_x];
				y_side = g_is_north(orientation) ? &cell->n : &cell->s;
				y_void_edge = y_edge && !y_side->type;
			}
			if (y_void || y_side->type || y_void_edge)
			{
				// We hit something - invert the sloping operation to determine the exact position of the intercept
				const i16 y_cell_side = g_is_north(orientation) ? y_cell : y_cell + 1;
				float x_inv = x - dx;
				float y_inv = y - (dx * slope);
				x_inv += ((y_cell_side * CELLSIZE) - y_inv) / slope;
				y_inv = (float)(y_cell_side * CELLSIZE);

				g_intercept_type intercept_type = G_INTERCEPT_VOID;
				if (!y_void_edge && y_side)
					intercept_type = y_side->flags & TRANSLUCENT ? G_INTERCEPT_NON_SOLID : G_INTERCEPT_SOLID;
				g_intercept intercept =
				{
					g_is_north(orientation) ? SIDE_ORIENTATION_NORTH : SIDE_ORIENTATION_SOUTH,
					intercept_type,
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
		const bool x_void = grid_x < 0 || grid_x >= map.boundsX;
		const bool x_edge =
			(g_is_west(orientation) && grid_x == 0) ||
			(g_is_east(orientation) && grid_x == map.boundsX - 1);
		bool x_void_edge = false;
		const Side* x_side = NULL;
		if (!x_void)
		{
			const Cell* cell = &map.cells[map.boundsX * grid_y + grid_x];
			x_side = g_is_east(orientation) ? &cell->e : &cell->w;
			x_void_edge = x_edge && !x_side->type;
		}
			
		if (x_void || x_side->type || x_void_edge)
		{
			// We hit something horizontally
			g_intercept_type intercept_type = G_INTERCEPT_VOID;
			if (x_void_edge)
				intercept_type = G_INTERCEPT_VOID_EDGE;
			else if (x_side)
				intercept_type = x_side->flags & TRANSLUCENT ? G_INTERCEPT_NON_SOLID : G_INTERCEPT_SOLID;
			g_intercept intercept =
			{
				g_is_east(orientation) ? SIDE_ORIENTATION_EAST : SIDE_ORIENTATION_WEST,
				intercept_type,
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
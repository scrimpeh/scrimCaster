#include <geometry.h>

#include <map/map.h>
#include <util/mathutil.h>

#include <math.h>

#define G_MAXSLOPE 1e+8f

void g_cast(float wx, float wy, angle_rad_f angle, g_intercept_collector intercept_collector, g_cell_collector cell_collector)
{
	i16 mx = (i16) floorf(wx / M_CELLSIZE);
	i16 my = (i16) floorf(wy / M_CELLSIZE);

	// Since our grid inverts the y coordinate, invert the slope
	g_orientation orientation = g_get_orientation(angle);
	const float slope = MATH_CAP(-G_MAXSLOPE, -tanf(angle), G_MAXSLOPE);
	float dx = (!g_is_east(orientation) ? mx : mx + 1) * M_CELLSIZE - wx;

	while (true)
	{
		wx += dx;
		wy += dx * slope;
		const i16 my_next = (i16) floorf(wy / M_CELLSIZE);

		// Trace grid vertically first
		for (i16 my_cur = my; my_cur != my_next; my_cur += g_is_north(orientation) ? -1 : 1)
		{
			// Out-of-bounds check
			if (my_cur < 0 || my_cur >= m_map.h || mx < 0 || mx >= m_map.w)
				return;

			if (cell_collector && !cell_collector(mx, my_cur))
				return;

			const m_side* side = m_get_side(mx, my_cur, g_is_north(orientation) ? M_NORTH : M_SOUTH);
			const bool edge = 
				(g_is_north(orientation) && my_cur == 0) || 
				(g_is_south(orientation) && my_cur == m_map.h - 1);

			if (intercept_collector && (edge || side->type))
			{
				// We hit something - invert the sloping operation to determine the exact position of the intercept
				const i16 my_side = g_is_north(orientation) ? my_cur : my_cur + 1;
				float wx_inv = wx - dx;
				float wy_inv = wy - dx * slope;
				wx_inv += (my_side * M_CELLSIZE - wy_inv) / slope;
				wy_inv = (float) (my_side * M_CELLSIZE);

				const u8 column = (u8) floorf(wx_inv) % M_CELLSIZE;
				g_intercept intercept;
				intercept.orientation = g_is_north(orientation) ? M_NORTH : M_SOUTH;
				intercept.type = g_get_intercept_type(side, edge);
				intercept.angle = angle;
				intercept.wx = wx_inv;
				intercept.wy = wy_inv;
				intercept.mx = mx;
				intercept.my = my_cur;
				intercept.column = g_is_north(orientation) ? column : M_CELLSIZE - 1 - column;

				// Add the intercept
				if (!intercept_collector(&intercept))
					return;
			}
		}

		// Now look horizontally
		my = my_next;

		// Out-of-bounds check
		if (my < 0 || my >= m_map.h || mx < 0 || mx >= m_map.w)
			return;

		if (cell_collector && !cell_collector(mx, my))
			return;

		const m_side* side = m_get_side(mx, my, g_is_east(orientation) ? M_EAST : M_WEST);
		const bool edge =
			(g_is_west(orientation) && mx == 0) ||
			(g_is_east(orientation) && mx == m_map.w - 1);
			
		if (intercept_collector && (edge || side->type))
		{
			const u8 column = (u8) floorf(wy) % M_CELLSIZE;
			g_intercept intercept;
			intercept.orientation = g_is_west(orientation) ? M_WEST : M_EAST;
			intercept.type = g_get_intercept_type(side, edge);
			intercept.angle = angle;
			intercept.wx = wx;
			intercept.wy = wy;
			intercept.mx = mx;
			intercept.my = my;
			intercept.column = g_is_east(orientation) ? column : M_CELLSIZE - 1 - column;

			// Add the intercept
			if (!intercept_collector(&intercept))
				return;
		}

		// Didn't find anything
		dx = g_is_east(orientation) ? (float) M_CELLSIZE : (float) -M_CELLSIZE;
		mx += g_is_east(orientation) ? 1 : -1;
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

static inline g_intercept_type g_get_intercept_type(const m_side* side, bool is_edge)
{
	g_intercept_type intercept_type = G_INTERCEPT_VOID;
	if (!is_edge)
		intercept_type = side->flags & TRANSLUCENT ? G_INTERCEPT_NON_SOLID : G_INTERCEPT_SOLID;
	return intercept_type;
}

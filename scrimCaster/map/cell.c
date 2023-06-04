#include <map/cell.h>

m_side* m_cell_get_side(m_cell* cell, m_orientation orientation)
{
	switch (orientation)
	{
	case M_EAST:  return &cell->e;
	case M_NORTH: return &cell->n;
	case M_WEST:  return &cell->w;
	case M_SOUTH: return &cell->s;
	default:
		SDL_assert(!"Invalid cell orientation!");
		return 0;
	}
}

m_orientation m_get_opposite_orientation(m_orientation orientation)
{
	switch (orientation)
	{
	case M_EAST:  return M_WEST;
	case M_NORTH: return M_SOUTH;
	case M_WEST:  return M_EAST;
	case M_SOUTH: return M_NORTH;

	case M_FLOOR: return M_CEIL;
	case M_CEIL:  return M_FLOOR;

	default:
		SDL_assert(!"Invalid cell orientation!");
		return 0;
	}
}

i16 m_get_next_x(i16 x, m_orientation orientation)
{
	switch (orientation)
	{
	case M_EAST:  return x + 1;
	case M_NORTH: return x;
	case M_WEST:  return x - 1;
	case M_SOUTH: return x;
	default:
		SDL_assert(!"Invalid cell orientation!");
		return 0;
	}
}

i16 m_get_next_y(i16 y, m_orientation orientation)
{
	switch (orientation)
	{
	case M_EAST:  return y;
	case M_NORTH: return y - 1;
	case M_WEST:  return y;
	case M_SOUTH: return y + 1;
	default:
		SDL_assert(!"Invalid cell orientation!");
		return 0;
	}
}
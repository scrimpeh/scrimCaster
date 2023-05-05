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
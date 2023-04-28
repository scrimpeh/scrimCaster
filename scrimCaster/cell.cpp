#include "cell.h"

Side* m_get_side(Cell* cell, m_orientation orientation)
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
#include <map/maputil.h>

#include <map/map.h>
#include <render/renderconstants.h>

#include <math.h>

Cell* map_get_cell(u16 grid_x, u16 grid_y)
{
	return &m_map.cells[grid_y * m_map.w + grid_x];
}

Side* map_get_side(u16 grid_x, u16 grid_y, m_orientation orientation)
{
	Cell* cell = map_get_cell(grid_x, grid_y);
	switch (orientation)
	{
	case M_EAST:
		return &cell->e;
	case M_NORTH:
		return &cell->n;
	case M_WEST:
		return &cell->w;
	case M_SOUTH:
		return &cell->s;
	default:
		SDL_assert(!"Invalid oreintation");
	}
}
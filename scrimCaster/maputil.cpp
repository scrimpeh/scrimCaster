#include "types.h"
#include "maputil.h"
#include "renderconstants.h"

#include "SDL/SDL_assert.h"
#include "SDL/SDL_log.h"
#include "math.h"


const float MAXSLOPE = 1e+8f;
extern Map map;

Cell* map_get_cell(u16 grid_x, u16 grid_y)
{
	return &map.cells[grid_y * map.w + grid_x];
}

Side* map_get_side(u16 grid_x, u16 grid_y, g_side_orientation orientation)
{
	Cell* cell = map_get_cell(grid_x, grid_y);
	switch (orientation)
	{
	case SIDE_ORIENTATION_NORTH:
		return &cell->n;
	case SIDE_ORIENTATION_WEST:
		return &cell->w;
	case SIDE_ORIENTATION_SOUTH:
		return &cell->s;
	case SIDE_ORIENTATION_EAST:
		return &cell->e;
	}
}
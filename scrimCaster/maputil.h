#pragma once

#include <common.h>

#include <map.h>
#include <geometry.h>

Cell* map_get_cell(u16 grid_x, u16 grid_y);
Side* map_get_side(u16 grid_x, u16 grid_y, g_side_orientation orientation);
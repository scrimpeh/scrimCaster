#pragma once

#include <common.h>

#include <geometry.h>
#include <map/map.h>

Cell* map_get_cell(u16 grid_x, u16 grid_y);
Side* map_get_side(u16 grid_x, u16 grid_y, m_orientation orientation);
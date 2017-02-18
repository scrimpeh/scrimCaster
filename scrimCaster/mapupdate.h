#pragma once

#include "map.h"

typedef struct SideList
{
	Side* side;
	SideList* next;
} CellList;

void UpdateSides(u32 timeStep);
SideList* AddActiveSide(Side* s);
void RemoveActiveSide(SideList* curr, SideList* prev);
void ClearActiveSides();

static bool UpdateDoor(Side* const side, u32 timeStep);

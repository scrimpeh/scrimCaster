#pragma once

#include "SDL/SDL_surface.h"
#include "cell.h"
#include "actor.h"

//Buffer for translucent sides.
typedef struct DrawSide
{
	const Side* side;
	float dist;
	u8 texcol;
	i8 layer;
} DrawSide;

static inline DrawSide MakeDrawSide(const Side* s, float p_x, float p_y, u8 orientation, i8 layer);

i32 InitializeScan(u8 collumn_width = 1);
void CloseScan();

void Scan(SDL_Surface* toDraw);
static void DrawCollumn(SDL_Surface* toDraw, const DrawSide ds, float angle, u16 col);
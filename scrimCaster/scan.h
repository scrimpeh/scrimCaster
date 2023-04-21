#pragma once

#include "common.h"

#include "SDL/SDL_surface.h"

#include "cell.h"
#include "actor.h"

//Buffer for translucent sides.
typedef struct DrawSide
{
	struct DrawSide* next;
	const Side* side;
	float dist;
	u8 texcol;
} DrawSide;

static inline void PushDrawSide(const Side* s, float p_x, float p_y, u8 orientation);

i32 InitializeScan(u8 collumn_width = 1);
void CloseScan();

void DrawGeometry(SDL_Surface* toDraw);
static void DrawColumn(SDL_Surface* toDraw, const DrawSide* ds, float angle, u16 col);
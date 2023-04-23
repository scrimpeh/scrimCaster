#pragma once

#include "common.h"

#include "geometry.h"
#include "cell.h"
#include "actor.h"

#include "SDL/SDL_surface.h"

i32 scan_init();
void scan_close();

void scan_draw(SDL_Surface* target);

static void scan_draw_column(SDL_Surface* target, float x, float y, const g_intercept* intercept, u16 col);

static inline bool collect_intercept(const g_intercept* intercept);

typedef struct g_intercept_stack
{
	g_intercept_stack* next;
	g_intercept intercept;
} g_intercept_stack;
#pragma once

#include <common.h>

#include <game/actor/actor.h>
#include <geometry.h>
#include <map/cell.h>
#include <render/color/colormap.h>

#include <SDL/SDL_surface.h>

typedef struct g_intercept_stack
{
	struct g_intercept_stack* next;
	g_intercept intercept;
} g_intercept_stack;

void scan_draw(SDL_Surface* target);

static void scan_draw_column(SDL_Surface* target, float x, float y, const g_intercept* intercept, u16 col);

static inline bool collect_intercept(const g_intercept* intercept);

static u8 scan_get_tx_slice_y(i64 wall_h, i64 y, u8 start_y);
static u8 scan_get_slice_y_start(const m_side* side);

static cm_color scan_get_flat_px(i16 mx, i16 my, u8 cx, u8 cy, m_orientation orientation);
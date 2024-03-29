#pragma once

#include <common.h>

// Responsible for drawing the map

#include <game/actor/actor.h>
#include <geometry.h>
#include <render/color/colormap.h>

#include <SDL/SDL_video.h>

void am_init();
void am_draw(SDL_Surface* target);

static void am_draw_map(SDL_Surface* target);
static void am_draw_cell(SDL_Surface* target, i16 x, i16 y);
static void am_draw_side(SDL_Surface* target, i16 x, i16 y, const m_side* side, m_orientation orientation);
static void am_draw_actors(SDL_Surface* target);
static void am_draw_actor(SDL_Surface* target, ac_actor* actor);

static i32 am_map_distance(float d);
static bool am_collect_intercept(const g_intercept* intercept);

static i32 am_map_h(float x);
static i32 am_map_v(float y);

static cm_color am_get_color(const m_side* side);
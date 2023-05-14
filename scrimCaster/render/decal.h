#pragma once

// A decal is a sprite that can be painted onto a wall or floor/ceiling texture.
// They may be rotated, but their bounds are always axis-aligned

// A decal may optionally overlap multiple sides, though certain sides, e.g., transparent sides
// cannot have decals on them.

// Decals are stored in two arrays. One is initialized at map load times and contains all fixed
// decals of the map. The other array is a buffer for dynamic decals, which acts as a ring buffer.
// I could also consider giving decals a time-to-live which decrements naturally.

#include <common.h>

#include <map/cell.h>

#include <SDL/SDL_video.h>

typedef struct {
	u16 sheet;
	u16 x;
	u16 y;
	u16 w;
	u16 h;
} r_decal_static;

typedef struct {
	u16 type;
	m_side_id id;
	u8 x;
	u8 y;
	u32 ttl;
} r_decal_world;

extern u16 r_decal_dynamic_max;


i32 r_decal_load();
void r_decal_unload();

void r_decal_draw(SDL_Surface* target);
void r_decal_add_dynamic(const r_decal_world* decal);

static void r_decal_draw_decal(SDL_Surface* target, const r_decal_world* decal);

static void r_decal_clear(r_decal_world* decal);
static bool r_decal_visible(const r_decal_world* decal);
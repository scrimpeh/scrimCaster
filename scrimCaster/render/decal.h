#pragma once

// A decal is a sprite that can be painted onto a wall or floor/ceiling texture.
// They may be rotated, but their bounds are always axis-aligned

// A decal may optionally overlap multiple sides, though certain sides, e.g., transparent sides
// cannot have decals on them.

// Decals are stored in two arrays. One is initialized at map load times and contains all fixed
// decals of the map. The other array is a buffer for dynamic decals, which uses a system of decal
// slots to keep track of free spaces

#include <common.h>

#include <map/cell.h>
#include <render/color/colormap.h>

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
	i32 ttl;
} r_decal_world;

typedef struct {
	u16 x_a;
	u16 y_a;
	u16 x_b;
	u16 y_b;
} r_decal_bounds;

typedef struct {
	i16 x;
	i16 y;
} r_decal_pt;

extern u16 r_decal_dynamic_max;

extern r_decal_world* r_decals_map;
extern r_decal_world* r_decals_dynamic;

i32 r_decal_load();
void r_decal_unload();

void r_decal_update(u32 delta);

void r_decal_get_map_bounds(const r_decal_world* decal, i32* bounds);
i16 r_decal_get_col(const r_decal_world* decal, i16 mx, i16 my, u8 side_col);
r_decal_pt r_decal_get_pt(const r_decal_static* decal, i32 wx, i32 wy);
bool r_decal_in_bounds(const r_decal_world* decal, r_decal_pt pt);

void r_decal_add_dynamic(const r_decal_world* decal);

static void r_decal_clear(r_decal_world* decal);

const r_decal_static* r_decal_get_static(const r_decal_world* world_decal);

const cm_color r_decal_get_px(const r_decal_static* decal, r_decal_pt pt);
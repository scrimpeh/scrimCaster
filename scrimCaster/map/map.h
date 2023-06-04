#pragma once

#include <common.h>

#include <game/actor/actorcontainers.h>
#include <map/cell.h>
#include <map/mapobject.h>
#include <render/decal.h>


typedef struct
{
	u16 w;
	u16 h;
	u16 sky;

	u8 texture_set_count;
	const char** texture_sets;

	m_cell* cells;

	u16 obj_count;
	m_obj* objects;

	// TODO: The map should have a different data structure not part of "render/..."
	u16 decal_count;
	r_decal_world* decals;
} m_map_data;

typedef struct
{
	m_side_id* sides;
	u32 count;
} m_taglist;

void m_load();
void m_unload();

extern m_map_data m_map;

m_cell* m_get_cell(u16 x, u16 y);
m_side* m_get_side(u16 x, u16 y, m_orientation o);
m_side* m_get_opposite_side(u16 x, u16 y, m_orientation o);
m_cell* m_get_next_cell(u16 x, u16 y, m_orientation o);
m_side* m_get_side_from_id(m_side_id id);

m_taglist* m_get_tags(u32 target);

static i32 m_create_tags();
static void m_destroy_tags();

// Development function, flood-fills a floor with a certain type
typedef bool (*_m_flood_fill_action)(m_cell* cell, void* data);

static void _m_flood_fill(u16 x, u16 y, _m_flood_fill_action func, void* data);

static void _m_set_double_sided(u16 x, u16 y, m_orientation orientation, const m_side* side);

static bool _m_flood_fill_cb_floor(m_cell* cell, void* data);
static bool _m_flood_fill_cb_ceil(m_cell* cell, void* data);
static bool _m_flood_fill_cb_brightness(m_cell* cell, void* data);
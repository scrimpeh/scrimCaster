#pragma once

// The blockmap is the geometric datastructure of choice for the game. It is a regular
// grid aligned with the map's cell grid that holds references to actors and decals for
// fast-ish retrieval.

// When rendering, I can, e.g., "light" up the blockmap entries I encountered to quickly find sprites

// A blockmap entry holds an entry even if the actor just partially intersects that entry.
// To ensure that entries aren't discovered multiple times, an entry remembers its positions in the blockmap

#include <common.h>

#include <game/actor/actor.h>
#include <render/decal.h>

typedef enum
{
	BLOCK_TYPE_ACTOR,
	BLOCK_TYPE_DECAL_FLAT,
	BLOCK_TYPE_DECAL_SIDE
} block_type;

typedef struct
{
	i16 x;
	i16 y;
} block_pt;

typedef struct
{
	union
	{
		r_decal_world* decal;
		ac_actor* actor;
	} reference;
	block_pt ne;
	block_pt nw;
	block_pt sw;
	block_pt se;
	block_type type;
} block_reference;

typedef struct block_ref_list_entry
{
	block_reference entry;
	struct block_ref_list_entry* prev;
	struct block_ref_list_entry* next;
} block_ref_list_entry;

typedef struct 
{
	u32 size;
	block_ref_list_entry* first;
	block_ref_list_entry* last;
} block_ref_list;

i32 block_load_map();
void block_unload();

void block_fill();

static void block_map_list_free(block_ref_list* list);

static void block_enter_actor(const ac_actor* actor);
static void block_enter_actor_point(const ac_actor* actor, block_pt* pts, u8 current);
bool block_pt_eq(block_pt a, block_pt b);

block_ref_list* block_ref_list_get(const block_pt pt);
void block_set_actor_points(const ac_actor* actor, block_pt* pts);
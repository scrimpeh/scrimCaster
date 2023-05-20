#include <map/block/blockmap.h>

#include <game/gameobjects.h>
#include <map/map.h>

static block_ref_list_triplet* block_map = NULL;
static u64 block_map_size = 0;

static block_ref_list** block_map_used_entries = NULL;
static u64 block_map_used_entries_count = 0;

i32 block_load_map()
{
	block_unload();

	block_map_size = m_map.w * m_map.h;

	block_map_used_entries = SDL_malloc(sizeof(block_ref_list*) * block_map_size);
	if (!block_map_used_entries)
		return -1;
	block_map_used_entries_count = 0;

	block_map = SDL_malloc(sizeof(block_ref_list_triplet) * block_map_size);
	if (!block_map)
	{
		SDL_free(block_map_used_entries);
		return -1;
	}
	for (u32 i = 0; i < block_map_size; i++)
	{
		block_map[i].actors.first = NULL;
		block_map[i].actors.last = NULL;
		block_map[i].actors.size = 0;

		block_map[i].side_decals.first = NULL;
		block_map[i].side_decals.last = NULL;
		block_map[i].side_decals.size = 0;

		block_map[i].flat_decals.first = NULL;
		block_map[i].flat_decals.last = NULL;
		block_map[i].flat_decals.size = 0;
	}

	return 0;
}

void block_unload()
{
	for (u32 i = 0; i < block_map_size; i++)
	{
		block_map_list_free(&block_map[i].actors);
		block_map_list_free(&block_map[i].flat_decals);
		block_map_list_free(&block_map[i].side_decals);
	}
	SDL_free(block_map);
	block_map = NULL;
	SDL_free(block_map_used_entries);
	block_map_used_entries = NULL;
	block_map_size = 0;
	block_map_used_entries_count = 0;
}

// TODO: I am not sure if this should be rebuilt each frame, or dynamically adjusted as actors move
void block_fill()
{
	for (u32 i = 0; i < block_map_used_entries_count; i++)
		block_map_list_free(block_map_used_entries[i]);
	block_map_used_entries_count = 0;

	// Enter actors in blockmap
	ac_list_node* cur_actor_node = ac_actors.first;
	while (cur_actor_node)
	{
		block_enter_actor(&cur_actor_node->actor);
		cur_actor_node = cur_actor_node->next;
	}

	// Enter decals in blockmap
	for (u32 i = 0; i < m_map.decal_count; i++)
		block_enter_decal(&r_decals_map[i]);
	for (u32 i = 0; i < r_decal_dynamic_max; i++)
		block_enter_decal(&r_decals_dynamic[i]);
}

void block_get_actor_rect(const ac_actor* actor, block_rect* rect)
{
	const ac_bounds bounds = ac_get_bounds(actor->type);
	rect->x_a = ((i64) actor->x - bounds.w) / M_CELLSIZE;
	rect->y_a = ((i64) actor->y - bounds.h) / M_CELLSIZE;
	rect->x_b = ((i64) actor->x + bounds.w) / M_CELLSIZE;
	rect->y_b = ((i64) actor->y + bounds.h) / M_CELLSIZE;
}

static void block_enter_actor(const ac_actor* actor)
{
	block_rect rect;
	block_get_actor_rect(actor, &rect);

	block_pt ne = block_get_pt(rect.x_b, rect.y_a);
	block_pt nw = block_get_pt(rect.x_a, rect.y_a);
	block_pt sw = block_get_pt(rect.x_a, rect.y_b);
	block_pt se = block_get_pt(rect.x_b, rect.y_b);

	block_enter_point(actor, BLOCK_TYPE_ACTOR, &rect, ne);
	if (!block_pt_eq(nw, ne))
		block_enter_point(actor, BLOCK_TYPE_ACTOR, &rect, nw);
	if (!block_pt_eq(sw, nw) && !block_pt_eq(sw, ne))
		block_enter_point(actor, BLOCK_TYPE_ACTOR, &rect, sw);
	if (!block_pt_eq(se, sw) && !block_pt_eq(se, nw) && !block_pt_eq(se, ne))
		block_enter_point(actor, BLOCK_TYPE_ACTOR, &rect, se);
}

static void block_enter_decal(const r_decal_world* decal)
{
	if (!decal->type)
		return;

	const r_decal_static* static_decal = r_decal_get_static(decal);
	block_rect rect;
	block_type type;

	switch (decal->id.orientation)
	{
	case M_CEIL:
	case M_FLOOR:
	{
		const i32 wx = decal->id.x * M_CELLSIZE + decal->x;
		const i32 wy = decal->id.y * M_CELLSIZE + decal->y;
		rect.x_a = (wx - (static_decal->w / 2)) / M_CELLSIZE;
		rect.y_a = (wy - (static_decal->h / 2)) / M_CELLSIZE;
		rect.x_b = (wx + (static_decal->w / 2)) / M_CELLSIZE;
		rect.y_b = (wy + (static_decal->h / 2)) / M_CELLSIZE;
		type = BLOCK_TYPE_DECAL_FLAT;
		break;
	}
	case M_NORTH:
	case M_SOUTH:
	{
		const i32 wx_offs = decal->id.orientation == M_NORTH ? decal->x : M_CELLSIZE - 1 - decal->x;
		const i32 wx = decal->id.x * M_CELLSIZE + wx_offs;
		rect.x_a = (wx - (static_decal->w / 2)) / M_CELLSIZE;
		rect.y_a = decal->id.y;
		rect.x_b = (wx + (static_decal->w / 2)) / M_CELLSIZE;
		rect.y_b = decal->id.y;
		type = BLOCK_TYPE_DECAL_SIDE;
		break;
	}
	case M_EAST:
	case M_WEST:
	{
		const i32 wy_offs = decal->id.orientation == M_EAST ? decal->x : M_CELLSIZE - 1 - decal->x;
		const i32 wy = decal->id.y * M_CELLSIZE + wy_offs;
		rect.x_a = decal->id.x;
		rect.y_a = (wy - (static_decal->w / 2)) / M_CELLSIZE;
		rect.x_b = decal->id.x;
		rect.y_b = (wy - (static_decal->w / 2)) / M_CELLSIZE;
		type = BLOCK_TYPE_DECAL_SIDE;
		break;
	}
	}

	block_pt ne = block_get_pt(rect.x_b, rect.y_a);
	block_pt nw = block_get_pt(rect.x_a, rect.y_a);
	block_pt sw = block_get_pt(rect.x_a, rect.y_b);
	block_pt se = block_get_pt(rect.x_b, rect.y_b);

	block_enter_point(decal, type, &rect, ne);
	if (!block_pt_eq(nw, ne))
		block_enter_point(decal, type, &rect, nw);
	if (!block_pt_eq(sw, nw) && !block_pt_eq(sw, ne))
		block_enter_point(decal, type, &rect, sw);
	if (!block_pt_eq(se, sw) && !block_pt_eq(se, nw) && !block_pt_eq(se, ne))
		block_enter_point(decal, type, &rect, se);
}


static void block_enter_point(const void* ref, block_type* type, const block_rect* rect, block_pt pt)
{
	if (pt.x < 0 || pt.x >= m_map.w || pt.y < 0 || pt.y >= m_map.h)
		return;

	block_ref_list* ref_list = block_ref_list_get(pt, type);

	block_ref_list_entry* new_entry = SDL_malloc(sizeof(block_ref_list_entry));
	new_entry->reference = ref;
	new_entry->pts = *rect;
	new_entry->next = NULL;
	new_entry->prev = ref_list->last;
	ref_list->last = new_entry;
	if (!ref_list->first)
		ref_list->first = new_entry;
	else
		new_entry->prev->next = new_entry;

	if (ref_list->size == 0)
		block_map_used_entries[block_map_used_entries_count++] = ref_list;

	ref_list->size++;
}

static void block_map_list_free(block_ref_list* list)
{
	block_ref_list_entry* cur = list->first;
	while (cur)
	{
		block_ref_list_entry* to_free = cur;
		cur = cur->next;
		SDL_free(to_free);
	}

	list->first = NULL;
	list->last = NULL;
	list->size = 0;
}

block_ref_list* block_ref_list_get(block_pt pt, block_type type)
{
	block_ref_list_triplet* point = &block_map[pt.y * m_map.w + pt.x];
	switch (type)
	{
	case BLOCK_TYPE_ACTOR:      return &point->actors;
	case BLOCK_TYPE_DECAL_SIDE: return &point->side_decals;
	case BLOCK_TYPE_DECAL_FLAT: return &point->flat_decals;
	}
}

bool block_pt_eq(block_pt a, block_pt b)
{
	return a.x == b.x && a.y == b.y;
}

block_pt block_get_pt(i16 mx, i16 my)
{
	block_pt pt;
	pt.x = mx;
	pt.y = my;
	return pt;
}
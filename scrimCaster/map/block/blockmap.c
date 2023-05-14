#include <map/block/blockmap.h>

#include <game/gameobjects.h>
#include <map/map.h>

static block_ref_list* block_map = NULL;
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

	block_map = SDL_malloc(sizeof(block_ref_list) * block_map_size);
	if (!block_map)
	{
		SDL_free(block_map_used_entries);
		return -1;
	}
	for (u32 i = 0; i < block_map_size; i++)
	{
		block_map[i].first = NULL;
		block_map[i].last = NULL;
		block_map[i].size = 0;
	}

	return 0;
}

void block_unload()
{
	for (u32 i = 0; i < block_map_size; i++)
		block_map_list_free(&block_map[i]);
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
}

static void block_enter_actor(const ac_actor* actor)
{
	block_pt pts[4];	// NE, NW, SW, SE
	block_set_actor_points(actor, pts);

	block_enter_actor_point(actor, pts, 0);
	if (!block_pt_eq(&pts[1], &pts[0]))
		block_enter_actor_point(actor, pts, 1);
	if (!block_pt_eq(&pts[2], &pts[1]) && !block_pt_eq(&pts[2], &pts[0]))
		block_enter_actor_point(actor, pts, 2);
	if (!block_pt_eq(&pts[3], &pts[2]) && !block_pt_eq(&pts[3], &pts[1]) && !block_pt_eq(&pts[3], &pts[0]))
		block_enter_actor_point(actor, pts, 3);
}

static void block_enter_actor_point(const ac_actor* actor, block_pt* pts, u8 current)
{
	block_ref_list* ref_list = block_ref_list_get(pts[current]);

	if (pts[current].x < 0 || pts[current].x >= m_map.w || pts[current].y < 0 || pts[current].y >= m_map.h)
		return;

	block_ref_list_entry* new_entry = SDL_malloc(sizeof(block_ref_list_entry));
	new_entry->entry.reference.actor = actor;
	new_entry->entry.type = BLOCK_TYPE_ACTOR;
	new_entry->entry.ne = pts[0];
	new_entry->entry.nw = pts[1];
	new_entry->entry.sw = pts[2];
	new_entry->entry.se = pts[3];
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

block_ref_list* block_ref_list_get(const block_pt pt)
{
	return &block_map[pt.y * m_map.w + pt.x];
}

void block_set_actor_points(const ac_actor* actor, block_pt* pts)
{
	const i64 x = (i64) actor->x;
	const i64 y = (i64) actor->y;
	const ac_bounds bounds = ac_get_bounds(actor->type);
	// NE, NW, SW, SE
	for (u8 i = 0; i < 4; i++)
	{
		pts[i].x = (x + bounds.w * (i == 0 || i == 3 ? 1 : -1)) / M_CELLSIZE;
		pts[i].y = (y + bounds.h * (i == 2 || i == 3 ? 1 : -1)) / M_CELLSIZE;
	}
}

bool block_pt_eq(const block_pt* a, const block_pt* b)
{
	return a->x == b->x && a->y == b->y;
}

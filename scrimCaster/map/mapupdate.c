#include <map/mapupdate.h>

#include <render/lighting/lighting.h>

#define DOOR_SCROLL_MAX (M_CELLHEIGHT - 4)
#define DOOR_TICKS_PER_SEC 60
#define PLAYER_HEIGHT 32

#define DOOR_OPEN (TRANSLUCENT)
#define DOOR_PASSABLE (BULLETS_PASS | PASSABLE)
#define DOOR_CLOSED ~DOOR_OPEN
#define DOOR_IMPASSABLE ~DOOR_PASSABLE

extern bool* m_tag_active;
extern u32 m_max_tag;

m_active_tag_list* m_active_tags = NULL;


void mu_update(u32 t_delta)
{
	m_active_tag_list* cur_tag = m_active_tags;
	m_active_tag_list* prev_tag = NULL;

	while (cur_tag)
	{
		bool all_done = true;
		m_taglist* sides = m_get_tags(cur_tag->tag);
		for (u32 i = 0; i < sides->count; i++)
			all_done &= mu_update_side(sides->sides[i], t_delta);

		if (all_done)
		{
			// All sides are done updating, remove the current tag
			m_tag_active[cur_tag->tag] = false;
			m_active_tag_list* to_free = cur_tag;
			cur_tag = cur_tag->next;
			if (prev_tag)
				prev_tag->next = cur_tag;
			else
				m_active_tags = cur_tag;
			SDL_free(cur_tag);
		}
		else
		{
			prev_tag = cur_tag;
			cur_tag = cur_tag->next;
		}
	}
}

void mu_activate_tag(u32 tag)
{
	SDL_assert(tag && tag <= m_max_tag);
	if (!m_tag_active[tag])
	{
		m_tag_active[tag] = true;
		m_active_tag_list* new_tag = SDL_malloc(sizeof(m_active_tag_list));
		new_tag->next = m_active_tags;
		new_tag->tag = tag;
		m_active_tags = new_tag;

		m_taglist* sides = m_get_tags(new_tag->tag);
		for (u32 i = 0; i < sides->count; i++)
			m_get_side_from_id(sides->sides[i])->state = 0;
	}
}

void mu_clear()
{
	for (u32 i = 0; i <= m_max_tag; i++)
		m_tag_active[i] = false;

	m_active_tag_list* cur = m_active_tags;
	m_active_tag_list* to_free = NULL;
	while (cur)
	{
		to_free = cur;
		cur = cur->next;
		SDL_free(to_free);
	}
}

static bool mu_update_side(m_side_id id, u32 delta)
{
	m_side* side = m_get_side_from_id(id);
	DoorParams* params = &side->door;
	const u8 ticks = params->timer_ticks + delta;

	switch (side->state)
	{
	default:
		SDL_assert(0);
		break;
	case 0:		// Activated
		side->flags = (m_side_flags)(side->flags | DOOR_OPEN);
		params->timer_ticks = 0;
		r_light_update(id.x - 1, id.y - 1, id.x + 1, id.y + 1);
		++side->state;
		return false;
	case 1:		// Opening
		params->scroll += ticks / params->openspeed;
		params->timer_ticks = ticks % params->openspeed;

		if (params->scroll > PLAYER_HEIGHT)
			side->flags = (m_side_flags)(side->flags | DOOR_PASSABLE);

		if (params->scroll > DOOR_SCROLL_MAX - 1)
		{
			if (params->staytime)
			{
				params->scroll = DOOR_SCROLL_MAX;
				params->timer_ticks = 0;
				params->timer_staycounter = params->staytime * DOOR_TICKS_PER_SEC;
				++side->state;
			}
			else
				side->state = 4;
		}
		return false;
	case 2:		//stay open
		params->timer_staycounter -= delta;
		if (params->timer_staycounter <= 0)
			++side->state;
		return false;
	case 3:
		params->scroll -= ticks / params->closespeed;
		params->timer_ticks = ticks % params->closespeed;

		if (params->scroll < PLAYER_HEIGHT)
			side->flags = (m_side_flags)(side->flags & DOOR_IMPASSABLE);

		if (params->scroll <= 0)
		{
			side->flags = (m_side_flags)(side->flags & DOOR_CLOSED);
			params->scroll = 0;
			++side->state;
			r_light_update(id.x - 1, id.y - 1, id.x + 1, id.y + 1);
			return false;
		}
		return false;
	case 4:
		return true;
	}
}

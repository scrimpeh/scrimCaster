#pragma once

#include <common.h>

#include <map/cell.h>
#include <map/map.h>

typedef struct m_active_tag_list
{
	u32 tag;
	struct m_active_tag_list* next;
} m_active_tag_list;

void mu_update(u32 t_delta);
void mu_activate_tag(u32 tag);
void mu_clear();

static bool mu_update_side(m_side_id id, u32 delta);

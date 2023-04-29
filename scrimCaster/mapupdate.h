#pragma once

#include "common.h"
#include "map.h"

typedef struct m_active_tag_list
{
	u32 tag;
	m_active_tag_list* next;
} m_active_tag_list;

void mu_update(u32 t_delta);
void mu_activate_tag(u32 tag);
void mu_clear();

static bool mu_update_side(Side* const side, u32 timeStep);

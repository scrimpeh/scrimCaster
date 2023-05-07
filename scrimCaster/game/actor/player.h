#pragma once

#include <common.h>

#include <game/actor/actor.h>
#include <map/mapobject.h>
#include <geometry.h>

void player_make(ac_actor* ac, m_obj* obj);
bool player_update(ac_actor* ac, u32 delta);

// Player functions
static void player_use();
static void player_fire();

static inline void player_set_movement(u32 delta);

static bool player_use_check_intercept(const g_intercept* intercept);
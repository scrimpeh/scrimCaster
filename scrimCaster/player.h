#pragma once

#include "common.h"

#include "actor.h"
#include "geometry.h"

extern Actor player;

void player_spawn();
void player_update(u32 delta);

// Player functions
static void player_use();
static void player_fire();

static inline void player_set_movement(u32 delta);

static bool player_use_check_intercept(const g_intercept* intercept);
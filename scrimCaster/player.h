#pragma once

#include "common.h"

#include "geometry.h"

void PlayerStartMap();
void UpdatePlayer(u32 delta);

// Player functions
static void player_use();
static void player_fire();

static inline void SetPlayerMovement(u32 delta);

static bool player_use_check_intercept(const g_intercept* intercept);
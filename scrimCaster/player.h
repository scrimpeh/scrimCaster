#pragma once

void PlayerStartMap();
void UpdatePlayer(u32 delta);

//Player functions
static void Use();
static void Fire();

inline void SetPlayerMovement(u32 delta);
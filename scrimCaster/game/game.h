#pragma once

#include <common.h>

i32 game_init();
void game_free();
void UpdateGame(u32 delta);
void SetMenu(bool open);
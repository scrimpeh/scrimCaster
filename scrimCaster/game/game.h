#pragma once

#include <common.h>

i32 game_init();
void game_free();
void game_update(u32 delta);
void game_pause(bool pause);
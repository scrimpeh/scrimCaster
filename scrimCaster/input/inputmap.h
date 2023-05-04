#pragma once

#include <common.h>

#include <SDL/SDL_keycode.h>

void input_map_load_bindings();
u8 input_map_get(SDL_Keycode kc);
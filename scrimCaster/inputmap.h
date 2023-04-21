#pragma once

#include "types.h"

#include "SDL/SDL_keycode.h"

void LoadInitialBindings();
extern inline u8 GetMapping(SDL_Keycode kc);
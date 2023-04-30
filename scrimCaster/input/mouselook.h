#pragma once

#include <common.h>

#include <SDL/SDL_events.h>

extern float mouselook_sensitivity;
extern bool mouselook_enable;
extern bool mouselook_is_suspended;

void mouselook_suspend(bool suspend);
void mouselook_set_properties(bool enable, float sens);
void mouselook_process_event(SDL_Event* evt);

#pragma once

#include "SDL/SDL_events.h"

void SuspendMouselook(bool suspend);
void SetMouselook(bool enable, float sens);
void ProcessMouseMotionEvent(SDL_Event* evt);

#pragma once

#include "types.h"

#include "SDL/SDL_events.h"

// First of all, a struct to hold all the player inputs for this frame:
typedef struct Input
{
	u8 _blank; // Used so that unmapped input will not write to forward
	u8 debug;  // Debug key

	u8 forward;
	u8 backward;
	u8 strafe_left;
	u8 strafe_right;
	u8 turn_left;
	u8 turn_right;

	u8 fire;
	u8 altfire;

	u8 pause;
	u8 use;
	u8 map;
} Input;

void InitializeInput();
void GetInput(SDL_Event* evt);
void FilterInput();
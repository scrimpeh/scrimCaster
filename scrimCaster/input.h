#pragma once
#include "SDL/SDL_events.h"

//First of all, a struct to hold all the player inputs for this frame:
typedef struct Input
{
private:
	u8 blank; //Used so that unmapped input will not write to forward
public:
	u8 forward, backward;
	u8 strafe_left, strafe_right;
	u8 turn_left, turn_right;
	u8 fire, altfire;
	u8 pause, use;
	u8 map;
} Input;

void InitializeInput();
void GetInput(SDL_Event* evt);
void FilterInput();
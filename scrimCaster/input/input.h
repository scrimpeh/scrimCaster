#pragma once

#include <common.h>

#include <SDL/SDL_events.h>

typedef struct
{
	u8 _blank; // Dummy value for unused keys to write to


	u8 forward;
	u8 backward;
	u8 strafe_left;
	u8 strafe_right;
	u8 turn_left;
	u8 turn_right;

	u8 fire;
	u8 altfire;
	u8 walk;

	u8 pause;
	u8 use;
	u8 map;
} input_keys;

extern input_keys input;
extern input_keys input_tf;

extern i64 input_mwheel;

void input_init();
void input_process_event(const SDL_Event* evt);
void input_filter();
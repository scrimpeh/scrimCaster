#include <input/input.h>
#include <input/inputmap.h>

#include <SDL/SDL_events.h>

input_keys input;
input_keys input_tf;
input_keys input_lf;

i64 input_mwheel_acc = 0;
i64 input_mwheel = 0;

void input_init() 
{
	SDL_Log("Loading key bindings...");
	input_map_load_bindings();
}

void input_process_event(const SDL_Event* evt)
{
	switch (evt->type)
	{
	case SDL_KEYDOWN:
	case SDL_KEYUP:
	{
		const u8 input_offset = input_map_get(evt->key.keysym.sym);
		*((u8*) &input + input_offset) = evt->key.state == SDL_PRESSED;
		break;
	}
	case SDL_MOUSEWHEEL:
	{
		input_mwheel_acc += evt->wheel.y;
		break;
	}
	}
}

void input_filter()
{
	// Filter old button presses
	input_tf = input_lf;
	u8* btn_ptr_tf = (u8*) &input_tf;
	u8* btn_ptr = (u8*) &input;

	for (u8 offs = 1; offs < sizeof(input_keys); offs++)
	{
		btn_ptr_tf[offs] ^= 0xFF;
		btn_ptr_tf[offs] &= btn_ptr[offs];
	}

	input_lf = input;

	input_mwheel = input_mwheel_acc;
	input_mwheel_acc = 0;
}
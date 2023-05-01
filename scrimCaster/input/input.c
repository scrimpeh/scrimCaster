#include <input/input.h>
#include <input/inputmap.h>

#include <SDL/SDL_events.h>

Input input;
Input input_tf;
Input input_lf;

i64 input_mwheel_acc = 0;
i64 input_mwheel = 0;

void InitializeInput() 
{
	SDL_Log("Loading key bindings...");
	LoadInitialBindings();
}

void GetInput(SDL_Event* evt)
{
	switch (evt->type)
	{
	case SDL_KEYDOWN:
	case SDL_KEYUP:
	{
		const u8 input_offset = GetMapping(evt->key.keysym.sym);
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

void FilterInput()
{
	// Filter old button presses
	input_tf = input_lf;
	u8* btn_ptr_tf = (u8*) &input_tf;
	u8* btn_ptr =    (u8*) &input;

	for (u8 offs = 1; offs < sizeof(Input); ++offs)
	{
		btn_ptr_tf[offs] ^= 0xFF;
		btn_ptr_tf[offs] &= btn_ptr[offs];
	}

	input_lf = input;

	input_mwheel = input_mwheel_acc;
	input_mwheel_acc = 0;
}
#include "inputmap.h"
#include "input.h"

//Inputmap's purpose is to map the actual processed window events to 
//input events - i.e. set keybindings

extern Input input;

#define  KEYCODE_COUNT 0x11A

//Maps SDL scan codes to teh offset in the input structure
u8 scancode_map[KEYCODE_COUNT];

void LoadInitialBindings()
{
	scancode_map[SDL_SCANCODE_LSHIFT] = offsetof(Input, debug);

	scancode_map[SDL_SCANCODE_W] = offsetof(Input, forward);
	scancode_map[SDL_SCANCODE_S] = offsetof(Input, backward);
	scancode_map[SDL_SCANCODE_A] = offsetof(Input, strafe_left);
	scancode_map[SDL_SCANCODE_D] = offsetof(Input, strafe_right);

	scancode_map[SDL_SCANCODE_Q] = offsetof(Input, turn_left);
	scancode_map[SDL_SCANCODE_LEFT] = offsetof(Input, turn_left);
	scancode_map[SDL_SCANCODE_E] = offsetof(Input, turn_right);
	scancode_map[SDL_SCANCODE_RIGHT] = offsetof(Input, turn_right);

	scancode_map[SDL_SCANCODE_ESCAPE] = offsetof(Input, pause);

	scancode_map[SDL_SCANCODE_SPACE] = offsetof(Input, use);
	scancode_map[SDL_SCANCODE_LCTRL] = offsetof(Input, fire);

	scancode_map[SDL_SCANCODE_TAB] = offsetof(Input, map);
} 

u8 GetMapping(SDL_Keycode kc)
{
	return scancode_map[SDL_GetScancodeFromKey(kc)];
}

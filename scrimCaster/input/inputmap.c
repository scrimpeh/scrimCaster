#include <input/inputmap.h>

#include <input/input.h>

#define INPUT_MAP_KEYCODE_COUNT 0x11A

// Maps SDL scan codes to offset in the input structure
static u8 input_map_scancodes[INPUT_MAP_KEYCODE_COUNT];

void input_map_load_bindings()
{
	input_map_scancodes[SDL_SCANCODE_W] = offsetof(input_keys, forward);
	input_map_scancodes[SDL_SCANCODE_S] = offsetof(input_keys, backward);
	input_map_scancodes[SDL_SCANCODE_A] = offsetof(input_keys, strafe_left);
	input_map_scancodes[SDL_SCANCODE_D] = offsetof(input_keys, strafe_right);

	input_map_scancodes[SDL_SCANCODE_Q] = offsetof(input_keys, turn_left);
	input_map_scancodes[SDL_SCANCODE_LEFT] = offsetof(input_keys, turn_left);
	input_map_scancodes[SDL_SCANCODE_E] = offsetof(input_keys, turn_right);
	input_map_scancodes[SDL_SCANCODE_RIGHT] = offsetof(input_keys, turn_right);

	input_map_scancodes[SDL_SCANCODE_ESCAPE] = offsetof(input_keys, pause);

	input_map_scancodes[SDL_SCANCODE_SPACE] = offsetof(input_keys, use);
	input_map_scancodes[SDL_SCANCODE_LCTRL] = offsetof(input_keys, fire);

	input_map_scancodes[SDL_SCANCODE_TAB] = offsetof(input_keys, map);
	input_map_scancodes[SDL_SCANCODE_LALT] = offsetof(input_keys, walk);

	input_map_scancodes[SDL_SCANCODE_PAUSE] = offsetof(input_keys, pause);
} 

u8 input_map_get(SDL_Keycode kc)
{
	return input_map_scancodes[SDL_GetScancodeFromKey(kc)];
}

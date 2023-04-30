#include <input/mouselook.h>

#include <actor.h>
#include <game.h>

float mouselook_sensitivity = 3.0;
bool mouselook_enable = false;
bool mouselook_is_suspended = true; 

extern Actor player;

void mouselook_suspend(bool suspend)
{
	SDL_SetRelativeMouseMode((SDL_bool) !suspend);
	mouselook_is_suspended = suspend;
}

void mouselook_set_properties(bool enable, float sens)
{
	if (sens >= 0) 
		mouselook_sensitivity = sens;
	mouselook_enable = enable;
	mouselook_is_suspended = !enable;
}

void mouselook_process_event(SDL_Event* evt) 
{
	SDL_assert(evt->type == SDL_MOUSEMOTION);

	if (mouselook_enable && !mouselook_is_suspended)
	{
		const double disp = (evt->motion.xrel / 24.) * mouselook_sensitivity;
		player.angle -= disp;
	}
}
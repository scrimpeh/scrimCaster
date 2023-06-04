#include <input/mouselook.h>

#include <game/actor/actor.h>
#include <game/gameobjects.h>
#include <util/mathutil.h>

float mouselook_sensitivity = 3.0;
bool mouselook_enable = false;
bool mouselook_is_suspended = true; 

#define MOUSELOOK_MIN 0.1
#define MOUSELOOK_MAX 10.0

void mouselook_suspend(bool suspend)
{
	SDL_SetRelativeMouseMode((SDL_bool) !suspend);
	mouselook_is_suspended = suspend;
}

void mouselook_set_properties(bool enable, float sens)
{
	mouselook_sensitivity = MATH_CAP(MOUSELOOK_MIN, sens, MOUSELOOK_MAX);
	mouselook_enable = enable;
	mouselook_is_suspended = !enable;
}

void mouselook_process_event(SDL_Event* evt) 
{
	SDL_assert(evt->type == SDL_MOUSEMOTION);
	ac_actor* player = ac_get_player();
	if (mouselook_enable && !mouselook_is_suspended)
	{
		const double disp = (evt->motion.xrel / 24.) * mouselook_sensitivity;
		player->angle -= disp;
	}
}
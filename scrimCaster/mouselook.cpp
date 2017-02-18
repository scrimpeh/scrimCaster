#include "types.h"
#include "mouselook.h"
#include "SDL/SDL_assert.h"

#include "actor.h"
#include "game.h"

float sensitivity = 3.0;
bool enable_mouselook = false;
bool suspend_mouselook = true; //used when out of focus or in menus

extern Actor player;

void SuspendMouselook(bool suspend)
{
	SDL_SetRelativeMouseMode((SDL_bool)!suspend);
	suspend_mouselook = suspend;
}

void SetMouselook(bool enable, float sens)
{
	if(sens >= 0) sensitivity = sens;
	enable_mouselook = enable;
	suspend_mouselook = !enable;
}

void ProcessMouseMotionEvent(SDL_Event* evt) 
{
	SDL_assert(evt->type == SDL_MOUSEMOTION);

	if (enable_mouselook && !suspend_mouselook)
	{
		const double disp = (evt->motion.xrel / 24.) * sensitivity;
		player.angle -= disp;
	}
}
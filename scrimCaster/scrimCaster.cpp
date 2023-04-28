#include "scrimCaster.h"
#include "SDL/SDL.h"

#include "init.h"
#include "cleanup.h"

#include "game.h"
#include "window.h"
#include "render.h"
#include "frame.h"
#include "input.h"
#include "mouselook.h"

// The great big 2023 revisit todo list

// [X] Fix sprites getting too large (Fix maybe ?)
// [X] Unified 2-D Z Buffer
// [X] Infinitely many draw sides
// [X] Make E/W walls slightly darker than N/S walls
// [ ] Distance fog
// [ ] Sprite rotation and animation system
// [ ] Sound (?)
// [ ] Decals (Dynamic and static)
// [X] Unified, general-purpose ray caster / geometry interceptor
// [ ] Code cleanup
// [ ] Port to C (C99 or newer)
// [ ] Update SDL to newest
// [ ] Reorganize source files to match VS structure
// [ ] Floor rendering (I think I might actually know how)
// [ ] Skybox texture with cylinder projection or similar
// [ ] HW Acceleration using SDL_Video

char* app_dir = NULL;

i32 main(i32 argc, char** argv)
{
	bool quit = false;
	u32 delta_ticks;
	u32 render_ticks;

	if (InitGame(argc, argv)) 
		return -1;

	SDL_Log("Starting Main Loop");
	delta_ticks = SDL_GetTicks();

	SDL_Event evt;
	while (!quit)
	{
		render_ticks = SDL_GetTicks();
		while (SDL_PollEvent(&evt))
		{
			switch (evt.type)
			{
			case SDL_MOUSEMOTION:
				ProcessMouseMotionEvent(&evt);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEWHEEL:
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				GetInput(&evt);
				break;
			case SDL_WINDOWEVENT:
				ProcessWindowEvent(&evt);
				break;
			case SDL_QUIT:
				SDL_Log("User Exit");
				quit = true;
				break;
			}
		}

		// Do game logic here
		u32 delta = SDL_GetTicks() - delta_ticks;
		FilterInput();
		UpdateGame(delta);
		delta_ticks = SDL_GetTicks();

		r_draw();
		frame_end(render_ticks);
	}

	CleanUp();
	return 0;
}
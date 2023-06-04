#include <scrimCaster.h>

#include <application.h>
#include <game/game.h>
#include <input/input.h>
#include <input/mouselook.h>
#include <render/frame.h>
#include <render/render.h>
#include <render/window.h>

#include <SDL/SDL.h>

// The great big 2023 revisit todo list

// [X] Fix sprites getting too large (Fix maybe ?)
// [X] Unified 2-D Z Buffer
// [X] Infinitely many draw sides
// [X] Make E/W walls slightly darker than N/S walls
// [X] Distance fog
// [X] Sprite rotation and animation system
// [ ] Sound (?)
// [X] Decals (Dynamic and static)
//     - [ ] Rotation / flip support
//     - [ ] Transparency
// [X] Unified, general-purpose ray caster / geometry interceptor
// [ ] Code cleanup
// [X] Port to C (C99 or newer)
// [ ] Update SDL to newest
// [X] Reorganize source files to match VS structure
// [X] Floor rendering (I think I might actually know how)
// [X] Skybox texture with cylinder projection or similar
// [ ] Optimize performance with the following methods (otpional)
//     - [ ] Multithread renderer
//     - [ ] Rotate surfaces by 90° so data is read and written sequentially
//     - [ ] Use SDL_render for scaling
//     - [ ] General optimizations
// [X] Cell lighting
//     - [X] Smooth light blending between adjacent cells
//     - [X] Dynamic lighting
// [ ] Add some proper textures
// [ ] Material system, with animated textures and more
// [X] Geometric data structure for sprite rendering, or maybe even marking all cells that we saw
// [X] Simplify all the actor and level object handling in general

i32 main(i32 argc, char** argv)
{
	bool quit = false;
	u32 delta_ticks;
	u32 render_ticks;

	if (app_init(argc, argv)) 
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
				mouselook_process_event(&evt);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEWHEEL:
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				input_process_event(&evt);
				break;
			case SDL_WINDOWEVENT:
				win_process_event(&evt);
				break;
			case SDL_QUIT:
				SDL_Log("User Exit");
				quit = true;
				break;
			}
		}

		// Do game logic here
		const u32 delta = SDL_GetTicks() - delta_ticks;
		input_filter();
		game_update(delta);
		delta_ticks = SDL_GetTicks();

		r_draw();
		frame_end(render_ticks);
	}

	app_close();
	return 0;
}
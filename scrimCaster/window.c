#include <window.h>

#include <input/mouselook.h>
#include <render.h>

#include <SDL/SDL.h>

const static SDL_WindowFlags win_modes[] = 
{
	SDL_WINDOW_SHOWN, 
	SDL_WINDOW_BORDERLESS, 
	SDL_WINDOW_FULLSCREEN 
};

SDL_Window* win_main = NULL;
SDL_PixelFormat* win_main_format = NULL;
i32 win_w = 1024;
i32 win_h = 768;
i32 win_x = SDL_WINDOWPOS_UNDEFINED;
i32 win_y = SDL_WINDOWPOS_UNDEFINED;
u32 win_mode = SDL_WINDOW_SHOWN;

extern bool suspend_mouselook;
extern bool inMenu;

extern SDL_Surface* r_surface_win;

i32 win_create()
{
	win_main = SDL_CreateWindow("scrimCaster", win_x, win_y,win_w, win_h, win_mode);
	r_surface_win = SDL_GetWindowSurface(win_main);
	win_main_format = r_surface_win->format;
	return !win_main;
}

void win_process_event(const SDL_Event* evt)
{
	SDL_assert(evt->type == SDL_WINDOWEVENT);
	switch (evt->window.event)
	{
	case SDL_WINDOWEVENT_FOCUS_GAINED:
		mouselook_suspend(inMenu);
		break;
	case SDL_WINDOWEVENT_FOCUS_LOST:
		mouselook_suspend(true);
		break;
	}
}

//Changes the Window Mode, returns 0 on success, 1 on error
//windowMode is either windowed border (0), windowed noborder (1) or fullscreen (2),
//in which case the other two parameters do not matter
i32 win_set_mode(i32 w, i32 h, win_display_mode mode)
{
	win_w = w;
	win_h = h;
	win_mode = win_modes[mode];

	// Recreate the window
	win_destroy();
	return win_create();
}

void win_destroy()
{
	SDL_DestroyWindow(win_main);
	win_main = NULL;
}
#include <common.h>

#include <SDL/SDL_events.h>

typedef enum
{
	WINDOW_MODE_NORMAL = 0,
	WINDOW_MODE_BORDERLESS = 1,
	WINDOW_MODE_FULLSCREEN = 2
} win_display_mode;

extern SDL_Window* win_main;

i32 win_create();
void win_process_event(const SDL_Event*);
i32 win_set_mode(i32 w, i32 h, win_display_mode mode);
void win_destroy();
#include "SDL/SDL_events.h"

typedef enum WindowMode : u32
{
	WINDOW_MODE_NORMAL = 0,
	WINDOW_MODE_BORDERLESS = 1,
	WINDOW_MODE_FULLSCREEN = 2
} WindowMode;

i32 CreateMainWindow();
void ProcessWindowEvent(const SDL_Event*);
i32 ChangeWindowMode(i32 newWidth, i32 newHeight, WindowMode newWindowMode);
void DestroyMainWindow();
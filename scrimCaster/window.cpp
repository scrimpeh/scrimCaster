#include "types.h"
#include "window.h"
#include "SDL/SDL.h"

#include "mouselook.h"
#include "render.h"

SDL_Window* mainWindow = NULL;
SDL_PixelFormat* mainWindowFmt = NULL;
i32 displayWidth = 1024;			//Subject to config files
i32 displayHeight = 768;
i32 windowPosX = SDL_WINDOWPOS_UNDEFINED;
i32 windowPosY = SDL_WINDOWPOS_UNDEFINED;
u32 windowMode = SDL_WINDOW_SHOWN;

extern bool suspend_mouselook;
extern bool inMenu;

extern SDL_Surface* mainWindowSurface;

i32 CreateMainWindow()
{
	mainWindow = SDL_CreateWindow("scrimCaster", 
		windowPosX, windowPosY,
		displayWidth, displayHeight,
		windowMode );
	
	mainWindowSurface = SDL_GetWindowSurface(mainWindow);
	mainWindowFmt = mainWindowSurface->format;
	
	return mainWindow == 0;
}

void ProcessWindowEvent(const SDL_Event* evt)
{
	SDL_assert(evt->type == SDL_WINDOWEVENT);
	switch (evt->window.event)
	{
	case SDL_WINDOWEVENT_FOCUS_GAINED:
		SuspendMouselook(inMenu);
		break;
	case SDL_WINDOWEVENT_FOCUS_LOST:
		SuspendMouselook(true);
		break;
	}
}

//Changes the Window Mode, returns 0 on success, 1 on error
//windowMode is either windowed border (0), windowed noborder (1) or fullscreen (2),
//in which case the other two parameters do not matter
i32 ChangeWindowMode(i32 newWidth, i32 newHeight, WindowMode newWindowMode)
{
	const static SDL_WindowFlags windowModes[] = {
		SDL_WINDOW_SHOWN, SDL_WINDOW_BORDERLESS, SDL_WINDOW_FULLSCREEN };

	auto newWinMode = (u32)newWindowMode;
	SDL_assert(newWinMode < SDL_arraysize(windowModes));

	displayWidth = newWidth;
	displayHeight = newHeight;
	windowMode = windowModes[newWinMode];

	//Now recreate the window
	DestroyMainWindow();
	return CreateMainWindow();
}

void DestroyMainWindow()
{
	SDL_DestroyWindow(mainWindow);
	mainWindow = NULL;
}
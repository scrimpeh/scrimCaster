#include "types.h"
#include "init.h"
#include "SDL\SDL.h"

#include "ttf.h"
#include "window.h"
#include "gfxloader.h"
#include "input.h"
#include "mouselook.h"
#include "render.h"
#include "camera.h"

#define VER_NUM "0.01"
#ifdef _MSC_VER
#define INTRO_MSG ("Starting scrimCaster v." VER_NUM ", compiled on: " __TIMESTAMP__)
#else
#define INTRO_MSG ("Starting scrimCaster v." VER_NUM ", compiled on: " __DATE__ ", " __TIME__)
#endif


extern char* app_dir;

//Returns 0 on success
i32 InitGame(i32 argc, char** argv)
{
	if (SDL_Init(SDL_INIT_EVERYTHING < 0))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fatal Error: Couldn't initialize SDL! %s", SDL_GetError());
		return -1;
	}

	SDL_Log(INTRO_MSG);
	SDL_Log("Loading Resources...");

	app_dir = SDL_GetBasePath();

	if (InitFonts())
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: Couldn't load fonts! You'll not see any text in the app! %s", SDL_GetError());
		//return -1;
	}

	SDL_Log("Trying to create window...");
	if (CreateMainWindow())
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fatal Error: Couldn't create window. %s", SDL_GetError());
		return -1;
	}

	SDL_Log("Loading surfaces...");
	LoadGlobalSurfaces();
	InitializeInput();
	SetMouselook(true, 2.0);
	SetViewportFov(90);

	if (InitializeRenderer(480, 360))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fatal Error: Can't initialize renderer! %s", SDL_GetError());
		return -1;
	}

	return 0;
}
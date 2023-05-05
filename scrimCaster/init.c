#include <init.h>

#include <camera.h>
#include <input/input.h>
#include <input/mouselook.h>
#include <map/map.h>
#include <render/gfxloader.h>
#include <render/render.h>
#include <render/ttf.h>
#include <render/window.h>

#include <SDL/SDL.h>


#define VER_NUM "0.02"
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

	if (ttf_init())
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: Couldn't load fonts! You'll not see any text in the app! %s", SDL_GetError());
		//return -1;
	}

	SDL_Log("Trying to create window...");
	if (win_create())
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fatal Error: Couldn't create window. %s", SDL_GetError());
		return -1;
	}

	SDL_Log("Loading surfaces...");
	if (gfx_load_global())
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fatal Error: Couldn't load global graphics. %s", SDL_GetError());
		return -1;
	}

	m_load();
	input_init();
	mouselook_set_properties(true, 2.0);
	SetViewportFov(90);

	if (r_init(256, 192))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fatal Error: Can't initialize renderer! %s", SDL_GetError());
		return -1;
	}

	return 0;
}
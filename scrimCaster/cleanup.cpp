#include "types.h"
#include "cleanup.h"
#include "SDL\SDL.h"

#include "window.h"
#include "render.h"
#include "map.h"
#include "gfxloader.h"

extern char* app_dir;

//Clean up everything
void CleanUp()
{
	r_close();
	UnloadMap();
	UnloadAllTextures();
	SDL_free(app_dir);
	DestroyMainWindow();

	SDL_Quit();
}
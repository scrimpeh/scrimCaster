#include <cleanup.h>

#include <map.h>
#include <render/gfxloader.h>
#include <render/render.h>
#include <render/window.h>

#include <SDL/SDL.h>

extern char* app_dir;

// Clean up everything
void CleanUp()
{
	r_close();
	m_unload();
	gfx_unload();
	SDL_free(app_dir);
	win_destroy();

	SDL_Quit();
}
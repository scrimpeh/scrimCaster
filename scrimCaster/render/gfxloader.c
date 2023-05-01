#include <gfxloader.h>

#include <renderconstants.h>
#include <render/skybox.h>

#include <SDL/SDL_image.h>

const char* SPR_PATH = "tx/spr/world/";

const char* WORLD_SPRITE_NAMES[] = 
{
	"lev.png"
};

extern char* app_dir;

#define WORLD_SPRITE_BUF_SIZE 1
SDL_Surface* gfx_ws_buffer[WORLD_SPRITE_BUF_SIZE] = 
{ 
	NULL
};

extern SDL_PixelFormat* win_main_format;

#define GFX_LOAD_PATH_BUF_SIZE 512
static char gfx_load_path_buf[GFX_LOAD_PATH_BUF_SIZE];

#define GFX_WS_LOAD_PATH_BUF_SIZE 512
static char gfx_ws_load_path_buf[GFX_WS_LOAD_PATH_BUF_SIZE];

SDL_Surface* gfx_load(const char* name)
{
	gfx_load_path_buf[0] = '\0';
	u32 pos = SDL_strlcat(gfx_load_path_buf, app_dir, GFX_LOAD_PATH_BUF_SIZE);
	SDL_strlcat(gfx_load_path_buf, name, GFX_LOAD_PATH_BUF_SIZE - pos);

	SDL_Surface* surf = IMG_Load(gfx_load_path_buf);
	if (!surf)
		return NULL;
	SDL_Surface* surf_optimized = SDL_ConvertSurface(surf, win_main_format, NULL);
	SDL_FreeSurface(surf);
	return surf_optimized;
}

i32 gfx_load_global()
{
	gfx_unload();
	if (gfx_load_sprites())
		return -1;
	if (r_sky_load_global())
		return -1;
	return 0;
}

static i32 gfx_load_sprites()
{
	for (u8 i = 0; i < WORLD_SPRITE_BUF_SIZE; i++)
	{
		gfx_ws_load_path_buf[0] = '\0';
		SDL_snprintf(gfx_ws_load_path_buf, GFX_WS_LOAD_PATH_BUF_SIZE, "%s%s", SPR_PATH, WORLD_SPRITE_NAMES[i]);

		const SDL_Surface* surf = gfx_load(gfx_ws_load_path_buf);
		if (surf)
			gfx_ws_buffer[i] = surf;
		else 
			return -1;
	}
	return 0;
};

void gfx_unload()
{
	// Free world sprites here
	for (u32 i = 0; i < WORLD_SPRITE_BUF_SIZE; i++)
	{
		SDL_FreeSurface(gfx_ws_buffer[i]);
		gfx_ws_buffer[i] = NULL;
	}
	r_sky_unload();
}



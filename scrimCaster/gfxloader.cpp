#include <gfxloader.h>
#include <renderconstants.h>
#include <texture.h>

#include <SDL/SDL_image.h>

const char* txPath = "tx/map/";
const char* sprPath = "tx/spr/world/";

const char* worldSpriteFilenames[] = 
{
	"lev.png"
};

extern char* app_dir;

//A surface buffer map texture files
//note that 256 is really optimistic and can probably be reduced
//by a lot
u8 fillCount = 0;						
const u8 MAX_TEXTURE_BUF = 16;
const u8 WORLD_SPRITE_BUF = SDL_arraysize(worldSpriteFilenames);
SDL_Surface* mapTextureBuffer[MAX_TEXTURE_BUF];
SDL_Surface* worldSpriteBuffer[WORLD_SPRITE_BUF];
						
char pathBuffer[256];

extern SDL_PixelFormat* win_main_format;

#define TX_LOAD_PATH_BUF_SIZE 512
static char tx_load_path_buf[TX_LOAD_PATH_BUF_SIZE];

SDL_Surface* gfx_load(const char* name)
{
	tx_load_path_buf[0] = '\0';
	u32 pos = SDL_strlcat(tx_load_path_buf, app_dir, TX_LOAD_PATH_BUF_SIZE);
	SDL_strlcat(tx_load_path_buf, name, TX_LOAD_PATH_BUF_SIZE - pos);

	SDL_Surface* surf = IMG_Load(tx_load_path_buf);
	if (!surf)
		return NULL;
	SDL_Surface* surf_optimized = SDL_ConvertSurface(surf, win_main_format, NULL);
	SDL_FreeSurface(surf);
	return surf_optimized;
}

i32 LoadGlobalSurfaces()
{
	SDL_memset(mapTextureBuffer, 0, sizeof(mapTextureBuffer));
	return LoadWorldSprites();
}

static i32 LoadWorldSprites()
{
	const u32 pathBufSize = sizeof(pathBuffer);
	SDL_Surface* surf = NULL;

	SDL_memset(worldSpriteBuffer, 0, sizeof(worldSpriteBuffer));
	for (u8 i = 0; i < WORLD_SPRITE_BUF; ++i)
	{
		SDL_memset(pathBuffer, '\0', pathBufSize);
		SDL_snprintf(pathBuffer, pathBufSize, "%s%s", sprPath, worldSpriteFilenames[i]);

		surf = gfx_load(pathBuffer);
		if (surf)
		{
			worldSpriteBuffer[i] = surf;
		}
		else return -1;
	}
	return 0;
};

//Returns the index of the buffer the surface was loaded into, or -1 if there was an error
i32 LoadMapTexture(const char* filename)
{
	SDL_Surface* surf = NULL;

	SDL_memset(pathBuffer, '\0', sizeof(pathBuffer));
	SDL_snprintf(pathBuffer, sizeof(pathBuffer), "%s%s", txPath, filename);
	const char* z = pathBuffer;

	if (fillCount < MAX_TEXTURE_BUF)
	{
		surf = gfx_load(pathBuffer);
		if (surf)
		{
			SDL_SetColorKey(surf, 1, COLOR_KEY);
			mapTextureBuffer[fillCount] = surf;
			return ++fillCount;
		}
	}
	return -1;
}

void UnloadMapTextures()
{
	tx_unload();
	for (u32 i = 0; i < MAX_TEXTURE_BUF; ++i)
		SDL_FreeSurface(mapTextureBuffer[i]);
	fillCount = 0;
}

void UnloadAllTextures()
{
	UnloadMapTextures();
}


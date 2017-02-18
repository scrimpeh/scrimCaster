#include "types.h"
#include "gfxloader.h"
#include "SDL/SDL_image.h"
#include "renderconstants.h"

const char* txPath = "tx\\map\\";
const char* sprPath = "tx\\spr\\world\\";

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

extern SDL_PixelFormat* mainWindowFmt;

//Try load the surface at the filename and load it to the destination buffer.
//Internal function, to be called by exported functions
static SDL_Surface* LoadSurface(const char* pathname)
{
	SDL_Surface *surf = NULL, *surf_opt = NULL;

	//Prepare filename
	surf = IMG_Load(pathname);

	if (surf)
		surf_opt = SDL_ConvertSurface(surf, mainWindowFmt, NULL);

	SDL_FreeSurface(surf);

	return surf_opt;
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
		SDL_snprintf(pathBuffer, pathBufSize, "%s%s%s", app_dir, sprPath, worldSpriteFilenames[i]);

		surf = LoadSurface(pathBuffer);
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
	SDL_snprintf(pathBuffer, sizeof(pathBuffer), "%s%s%s", app_dir, txPath, filename);
	const char* z = pathBuffer;

	if (fillCount < MAX_TEXTURE_BUF)
	{
		surf = LoadSurface(pathBuffer);
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
	for (u32 i = 0; i < MAX_TEXTURE_BUF; ++i)
		SDL_FreeSurface(mapTextureBuffer[i]);
	fillCount = 0;
}

void UnloadAllTextures()
{
	UnloadMapTextures();
}


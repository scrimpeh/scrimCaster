#include "types.h"
#include "ttf.h"
#include "SDL/SDL_ttf.h"

const char* fonDirPath = "fon\\";
const char* fonNames[] = { "vgasys.fon" };

extern char* app_dir;

char fonPathBuf[256];

TTF_Font* fontDebug = NULL;

u32 InitFonts()
{
	if (TTF_Init() < 0) return -1;
	
	SDL_memset(fonPathBuf, 0, sizeof(fonPathBuf));
	SDL_snprintf(fonPathBuf, sizeof(fonPathBuf), "%s%s%s", app_dir, fonDirPath, fonNames[0]);
	
	//NOTE: This routine is normally supposed to load a font from the app directory,
	//however, since I don't have any free font ready, and I can't just distribute Windows
	//fonts with the app, this just loads vgasys.fon from the Windows directory.
	//If, for some reason, you don't have vgasys.fon on your computer, use another font.

	//fontDebug = TTF_OpenFont(fonPathBuf, 12);
	fontDebug = TTF_OpenFont("C:\\Windows\\Fonts\\vgasys.fon", 12);

	return !fontDebug;
}

void CloseFonts()
{
	TTF_CloseFont(fontDebug);
	TTF_Quit();
}
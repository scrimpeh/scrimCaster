#include <render/ttf.h>

#include <SDL/SDL_ttf.h>

TTF_Font* ttf_font_debug = NULL;

#ifndef TX_FONT_DEBUG
#define TX_FONT_DEBUG "C:\\Windows\\Fonts\\vgasys.fon"
#endif

i32 ttf_init()
{
	if (TTF_Init() < 0) 
		return -1;

	ttf_font_debug = TTF_OpenFont(TX_FONT_DEBUG, 8);
	return !ttf_font_debug;
}

void ttf_close()
{
	TTF_CloseFont(ttf_font_debug);
	TTF_Quit();
}
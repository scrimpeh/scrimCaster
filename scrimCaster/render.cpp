#include "render.h"

#include "automap.h"
#include "camera.h"
#include "input.h"
#include "renderconstants.h"
#include "renderutil.h"
#include "scan.h"
#include "sprite.h"

#include "SDL/SDL_ttf.h"

// These are decoupled from the window width and height and should equal to or smaller than the window
u16 viewport_w;
u16 viewport_h;

extern i32 displayWidth;
extern i32 displayHeight;

extern TTF_Font* ttf_font_debug;

extern SDL_Window* mainWindow;
SDL_Surface* viewportSurface = NULL;
SDL_Surface* mainWindowSurface = NULL;

extern u64 framecounter;
extern float framerate;

extern ActorVector levelEnemies;

bool show_map = false;
bool draw_crosshair = true;

i32 r_init(u16 w, u16 h)
{
	r_close();	//close the old renderer first

	viewport_w = w;
	viewport_h = h;

	viewportSurface = SDL_CreateRGBSurface(0, viewport_w, viewport_h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000);
	if (!viewportSurface)
	{
		SDL_LogError(SDL_LOG_PRIORITY_CRITICAL, "Couldn't initialize viewport! %s", SDL_GetError());
		return -1;
	}

	if (scan_init())
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't initialize map renderer! %s", SDL_GetError());
		return -1;
	}

	am_init();

	return 0;
}

void r_close()
{
	scan_close();
	SDL_FreeSurface(viewportSurface);
	viewportSurface = NULL;
}

void r_draw_crosshair(SDL_Surface* target)
{
	const u16 center_x = r_hud_px_h(.5f);
	const u16 center_y = r_hud_px_v(.5f);
	const u16 length = r_hud_px_h(1.0f / 32);

	r_draw_line(target, center_x - length, center_y, center_x + length, center_y, CM_GET(0xFF, 0xFF, 0xFF));
	r_draw_line(target, center_x, center_y - length, center_x, center_y + length, CM_GET(0xFF, 0xFF, 0xFF));
}

void r_draw()
{
	// Do the rendering pipeline
	show_map = input.map;

	scan_draw(viewportSurface);
	DrawSprites(viewportSurface);
	if (show_map)
		am_draw(viewportSurface);
	else if (draw_crosshair) 
		r_draw_crosshair(viewportSurface);
	
	char buf[256];
	SDL_memset(buf, 0, 256);
	SDL_snprintf(buf, 256, "FPS: %.2f | Framecount: %d", framerate, framecounter);
	r_draw_text(viewportSurface, buf, 4, 2, ttf_font_debug, CM_GET(255, 255, 255));

	SDL_BlitScaled(viewportSurface, NULL, mainWindowSurface, NULL);
	
	SDL_UpdateWindowSurface(mainWindow);
}

i16 r_hud_px_h(float x)
{
	return (i16)(x * viewport_w);
}

i16 r_hud_px_v(float y)
{
	return (i16)(y * viewport_h);
}

float r_hud_hu_h(i16 x)
{
	return (float)x / viewport_x;
}

float r_hud_hu_v(i16 y)
{
	return (float)y / viewport_y;
}
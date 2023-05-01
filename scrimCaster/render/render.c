#include <render/render.h>

#include <camera.h>
#include <input/input.h>
#include <render/automap.h>
#include <render/renderconstants.h>
#include <render/renderdebug.h>
#include <render/renderutil.h>
#include <render/scan.h>
#include <render/sprite.h>

// These are decoupled from the window width and height and should equal to or smaller than the window
u16 viewport_w;
u16 viewport_h;
u8 viewport_x_fov;

extern TTF_Font* ttf_font_debug;

extern SDL_Window* win_main;
SDL_Surface* r_surface_viewport = NULL;
SDL_Surface* r_surface_win = NULL;

extern u64 frame_count;
extern float frame_fps;

bool r_show_map = false;
bool r_show_crosshair = true;
bool r_draw_background = false;

i32 r_init(u16 w, u16 h)
{
	r_close();	// Close the old renderer first

	viewport_w = w;
	viewport_h = h;

	r_surface_viewport = SDL_CreateRGBSurface(0, viewport_w, viewport_h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000);
	if (!r_surface_viewport)
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
	SDL_FreeSurface(r_surface_viewport);
	r_surface_viewport = NULL;
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
	r_show_map = input.m_map;


	// Optional HOM avoidance
	if (r_draw_background)
		SDL_FillRect(r_surface_viewport, NULL, COLOR_KEY);

	scan_draw(r_surface_viewport);
	DrawSprites(r_surface_viewport);
	if (r_show_map)
		am_draw(r_surface_viewport);
	else if (r_show_crosshair) 
		r_draw_crosshair(r_surface_viewport);
	
	rd_render_debug(r_surface_viewport);
	
	SDL_BlitScaled(r_surface_viewport, NULL, r_surface_win, NULL);
	SDL_UpdateWindowSurface(win_main);
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
#include <render/render.h>

#include <game/camera.h>
#include <input/input.h>
#include <render/automap.h>
#include <render/decal.h>
#include <render/lighting/lighting.h>
#include <render/renderconstants.h>
#include <render/renderdebug.h>
#include <render/renderutil.h>
#include <render/scan.h>
#include <render/sprite.h>
#include <render/viewport.h>

extern TTF_Font* ttf_font_debug;

extern SDL_Window* win_main;
SDL_Surface* r_surface_win = NULL;

extern u64 frame_count;
extern float frame_fps;

bool r_show_map = false;
bool r_show_crosshair = true;
bool r_draw_background = false;

i32 r_init(u16 w, u16 h)
{
	r_close();	// Close the old renderer first

	if (viewport_init(w, h))
	{
		SDL_LogError(SDL_LOG_PRIORITY_CRITICAL, "Couldn't initialize viewport! %s", SDL_GetError());
		return -1;
	}

	if (r_light_init(R_LIGHT_SMOOTH_FINE))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't initialize lighting! %s", SDL_GetError());
		return -1;
	}

	if (r_decal_load()) 
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Can't initialize decals! %s", SDL_GetError());
		return -1;
	}

	am_init();
	return 0;
}

void r_close()
{
	viewport_destroy();
	r_decal_unload();
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
	r_show_map = input.map;

	// Optional HOM avoidance
	if (r_draw_background)
		SDL_FillRect(viewport_surface, NULL, COLOR_KEY);

	scan_draw(viewport_surface);
	spr_draw(viewport_surface);
	r_decal_draw(viewport_surface);

	if (r_show_map)
		am_draw(viewport_surface);
	else if (r_show_crosshair) 
		r_draw_crosshair(viewport_surface);
	
	rd_render_debug(viewport_surface);
	
	SDL_BlitScaled(viewport_surface, NULL, r_surface_win, NULL);
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
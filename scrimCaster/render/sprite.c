#include <render/sprite.h>

#include <game/camera.h>
#include <map/block/blockmap.h>
#include <map/block/block_iterator.h>
#include <map/map.h>
#include <render/color/colorramp.h>
#include <render/gfxloader.h>
#include <render/lighting/lighting.h>
#include <render/renderconstants.h>
#include <render/scan.h>
#include <render/viewport.h>
#include <util/mathutil.h>

#include <math.h>

// Static Sprite definition
// TODO: Use textures instead and put each texture on its own frame
static const spr_actor_frame_map SPR_DEF[] =
{
	[AC_DUMMY] =
	{
		(spr_frameset[]) 
		{ 
			{ 
				1,
				(spr_frame[])
				{
					{ .sheet = 0, .x = 0, .y = 0, .w = 0, .h = 0, .anchor = SPR_CENTER, .offset = 0 },
				}
			}
		}
	},
	[AC_PLAYER] =
	{
		(spr_frameset[])
		{
			{
				1,
				(spr_frame[])
				{
					{ .sheet = 0, .x = 0, .y = 0, .w = 0, .h = 0, .anchor = SPR_CENTER, .offset = 0 },
				}
			}
		}
	},
	[AC_DUMMY_ENEMY] =
	{
		(spr_frameset[])
		{
			{
				1,
				(spr_frame[])
				{
					{ .sheet = 0, .x = 0, .y = 0, .w = 28, .h = 64, .anchor = SPR_CENTER, .offset = 0 },
				}
			}
		}
	},
	[AC_PILLAR] =
	{
		(spr_frameset[])
		{
			{
				1,
				(spr_frame[])
				{
					{ .sheet = 0, .x = 0, .y = 0, .w = 28, .h = 64, .anchor = SPR_FLOOR, .offset = 0 },
				}
			}
		}
	},
	[AC_T_LIGHT_FLICKER] =
	{
		(spr_frameset[])
		{
			{
				1,
				(spr_frame[])
				{
					{ .sheet = 0, .x = 0, .y = 0, .w = 0, .h = 0, .anchor = SPR_FLOOR, .offset = 0 },
				}
			}
		}
	}
};

void spr_draw(SDL_Surface* target)
{
	const ac_actor* ac = block_iterator_next(scan_sprite_iter);
	while (ac)
	{
		if (ac != cam_get_actor())
			spr_draw_actor(target, ac);
		ac = block_iterator_next(scan_sprite_iter);
	}
}

static void spr_draw_actor(SDL_Surface* target, const ac_actor* ac)
{
	// Find angle and distance to sprite
	const angle_rad_f slope = angle_get_rad_f(ac->x - viewport_x, -ac->y + viewport_y);
	const angle_rad_f angle = TO_RADF(viewport_angle) - slope;
	const float distance = math_dist_f(viewport_x, viewport_y, ac->x, ac->y) * cosf(angle);
	if (distance < 0)
		return;

	// Get drawing coordinates on screen
	const spr_frame* ws = spr_get_frame(ac);
	SDL_Rect spr_rect;
	spr_rect.w = viewport_distance_to_length(ws->w, distance);
	spr_rect.h = viewport_distance_to_length(ws->h, distance);
	spr_rect.x = viewport_angle_to_x(angle) - (spr_rect.w / 2);
	spr_rect.y = viewport_distance_to_y(distance, spr_get_y_offset(ws));

	const i32 x_start = SDL_max(spr_rect.x, 0);
	const i32 x_end = SDL_min(spr_rect.x + spr_rect.w, viewport_w);
	const float x_inc = ws->w / (float) spr_rect.w;

	const i32 y_start = SDL_max(spr_rect.y, 0);
	const i32 y_end = SDL_min(spr_rect.y + spr_rect.h, viewport_h);
	const float y_inc = ws->h / (float) spr_rect.h;

	// Draw the sprite column by column
	const u16 spritesheet_width = gfx_ws_buffer[ws->sheet]->w;
	const u32* spr_ptr_begin = (u32*) gfx_ws_buffer[ws->sheet]->pixels + (ws->y * spritesheet_width) + ws->x;

	float sprcol_x = spr_rect.x >= 0 ? 0 : -spr_rect.x * x_inc;

	const i16 map_x = (i16) floor(ac->x / M_CELLSIZE);
	const i16 map_y = (i16) floor(ac->y / M_CELLSIZE);
	const i16 cell_x = (i16) fmod(ac->x, M_CELLSIZE);
	const i16 cell_y = (i16) fmod(ac->y, M_CELLSIZE);

	const float brightness = r_light_get_alpha(map_x, map_y, ws->anchor == SPR_CEIL ? M_CEIL : M_FLOOR, cell_x, cell_y);
	const cm_alpha_color distance_fog = cm_ramp_get_px(distance);

	for (i32 x_i = x_start; x_i < x_end; ++x_i)
	{
		float sprcol_y = spr_rect.y >= 0 ? 0 : -spr_rect.y * y_inc;
		u32* render_px = (u32*) target->pixels + (y_start * viewport_w) + x_i;
		float* z_buffer_px = viewport_z_buffer + (y_start * viewport_w) + x_i;
		const u32* const ws_px = (u32*) spr_ptr_begin + (u16) sprcol_x;
		for (i32 y_i = y_start; y_i < y_end; ++y_i)
		{
			u32 spr_px = *(ws_px + (u16) sprcol_y * spritesheet_width);
			if (spr_px != COLOR_KEY && *z_buffer_px >= distance)
			{
				*render_px = cm_ramp_apply(r_light_apply(spr_px, brightness), distance_fog);
				*z_buffer_px = distance;
			}
			sprcol_y += y_inc;
			render_px += viewport_w;
			z_buffer_px += viewport_w;
		}

		sprcol_x += x_inc;
	}
}

static const spr_frame* spr_get_frame(const ac_actor* a)
{
	const spr_frameset* spr_frames = &SPR_DEF[a->type].frames[a->frame];
	const angle_d range_per_frame = 360. / spr_frames->angles;
	const angle_d relative_angle = angle_normalize_deg_d(range_per_frame / 2 + viewport_angle - a->angle);
	const u8 frame = floor(relative_angle / range_per_frame);
	return &spr_frames->sprites[frame];
}

static i32 spr_get_y_offset(const spr_frame* ws)
{
	switch (ws->anchor)
	{
	case SPR_CEIL:
		return ws->offset;
	case SPR_CENTER:
		return R_HALF_H - ws->h / 2;
	case SPR_FLOOR:
		return R_CELL_H - ws->offset - ws->h;
	}
}
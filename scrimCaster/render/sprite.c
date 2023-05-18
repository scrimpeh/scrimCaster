#include <render/sprite.h>

#include <game/camera.h>
#include <game/gameobjects.h>
#include <map/map.h>
#include <render/color/colorramp.h>
#include <render/lighting/lighting.h>
#include <render/renderconstants.h>
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
	}
};

#define MAX_SPRITES 256

static spr_actor sprite_slot_buffer[MAX_SPRITES];

extern SDL_Surface* gfx_ws_buffer[];

void spr_draw(SDL_Surface* target)
{
	SDL_Rect spr_rect;

	u32 ds_index = 0;
	const ac_list_node* node = ac_actors.first;
	while (node)
	{
		if (spr_is_visible(&node->actor, ds_index))
			if (++ds_index >= MAX_SPRITES)
				break;
		node = node->next;
	}

	while (ds_index--)
	{
		const spr_actor* ds = &sprite_slot_buffer[ds_index];
		if (ds->distance <= 0)
			continue;

		const spr_frame* ws = spr_get_frame(ds->actor);

		const angle_d angle = angle_normalize_deg_d(ds->angle - (viewport_x_fov / 2));

		spr_rect.w = viewport_distance_to_length(ws->w, ds->distance);
		spr_rect.h = viewport_distance_to_length(ws->h, ds->distance);
		spr_rect.x = viewport_angle_to_x(TO_RADF(angle)) - (spr_rect.w / 2);
		spr_rect.y = viewport_distance_to_y(ds->distance, spr_get_y_offset(ws));
			
		// The idea: Make a loop running through all the pixel columns, clipping accordingly at the edges:
		// If and only if the sprite needs to be masked, blit into a specialty area first
		const i32 x_start = SDL_max(spr_rect.x, 0);
		const i32 x_end = SDL_min(spr_rect.x + spr_rect.w, viewport_w);
		const float x_inc = ws->w / (float) spr_rect.w;

		const i32 y_start = SDL_max(spr_rect.y, 0);
		const i32 y_end = SDL_min(spr_rect.y + spr_rect.h, viewport_h);
		const float y_inc = ws->h / (float) spr_rect.h;

		const u16 spritesheet_width = gfx_ws_buffer[ws->sheet]->w;
		const u32* spr_ptr_begin = (u32*) gfx_ws_buffer[ws->sheet]->pixels + (ws->y * spritesheet_width) + ws->x;

		float sprcol_x = spr_rect.x >= 0 ? 0 : -spr_rect.x * x_inc;

		const i16 map_x = (i16) floor(ds->actor->x / M_CELLSIZE);
		const i16 map_y = (i16) floor(ds->actor->y / M_CELLSIZE);
		const i16 cell_x = (i16) fmod(ds->actor->x, M_CELLSIZE);
		const i16 cell_y = (i16) fmod(ds->actor->y, M_CELLSIZE);

		const float brightness = r_light_get_alpha(map_x, map_y, ws->anchor == SPR_CEIL ? M_CEIL : M_FLOOR, cell_x, cell_y);
		const cm_alpha_color distance_fog = cm_ramp_get_px(ds->distance);

		for (i32 x_i = x_start; x_i < x_end; ++x_i)
		{
			float sprcol_y = spr_rect.y >= 0 ? 0 : -spr_rect.y * y_inc;
			u32* render_px = (u32*) target->pixels + (y_start * viewport_w) + x_i;
			float* z_buffer_px = viewport_z_buffer + (y_start * viewport_w) + x_i;
			const u32* const ws_px = (u32*) spr_ptr_begin + (u16) sprcol_x;
			for (i32 y_i = y_start; y_i < y_end; ++y_i)
			{
				u32 spr_px = *(ws_px + (u16) sprcol_y * spritesheet_width);
				if (spr_px != COLOR_KEY && *z_buffer_px >= ds->distance)
				{
					*render_px = cm_ramp_apply(r_light_apply(spr_px, brightness), distance_fog);
					*z_buffer_px = ds->distance;
				}
				sprcol_y += y_inc;
				render_px += viewport_w;
				z_buffer_px += viewport_w;
			}
	
			sprcol_x += x_inc;
		}
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

static bool spr_is_visible(const ac_actor* actor, u32 ds_index)
{
	// Skip the current actor, usually the player
	if (actor == cam_get_actor())
		return false;

	// Calculate relative angle of the sprite to the viewport. If it is outside the viewport, skip it.
	// Since a sprite may be partially outside of the screen, we calculate an error angle from the sprite's size.
	const float dx = actor->x - viewport_x;
	const float dy = actor->y - viewport_y;
	const angle_f slope = angle_get_deg_f(dx, -dy);
	const angle_rad_f angle = TO_RADF(viewport_angle - slope);

	const float distance = math_dist_f(viewport_x, viewport_y, actor->x, actor->y);
	const float distance_corrected = distance * cosf(angle);

	const angle_f fov_min_half = angle_normalize_deg_f(viewport_angle - viewport_x_fov / 2);
	const angle_f fov_max_half = angle_normalize_deg_f(viewport_angle + viewport_x_fov / 2);

	spr_frame* ws = spr_get_frame(actor);
	const double spr_width = ((viewport_projection_distance * ws->w) / distance_corrected) / 2;
	const float px_angle_ratio = (float) viewport_x_fov / viewport_w;
	const angle_f err_angle = px_angle_ratio * spr_width;

	// The error angle now contains the absolute offset that we should look ahead
	// TODO: Flip the meaning of min and max, 'min' is at the left of the viewport, i.e. adding angle
	const angle_f fov_min_err = angle_normalize_deg_f(fov_min_half - err_angle);
	const angle_f fov_max_err = angle_normalize_deg_f(fov_max_half + err_angle);

	bool drawsprite;
	if (fov_min_err > fov_max_err)	// 360° rollaround
		drawsprite = slope >= fov_min_err || slope <= fov_max_err;
	else
		drawsprite = slope >= fov_min_err && slope <= fov_max_err;
	if (drawsprite)
	{
		sprite_slot_buffer[ds_index].actor = actor;
		sprite_slot_buffer[ds_index].distance = distance_corrected;
		sprite_slot_buffer[ds_index].angle = angle_normalize_deg_f(fov_max_half - slope);
		return true;
	}
	return false;
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
#include <render/sprite.h>

#include <game/camera.h>
#include <game/gameobjects.h>
#include <map/map.h>
#include <render/color/colorramp.h>
#include <render/lighting/lighting.h>
#include <render/renderconstants.h>
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

extern u16 viewport_w;
extern u16 viewport_h;

extern float projection_dist;

extern float* z_buffer;
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
		const spr_frame* ws = spr_get_frame(ds);

		const i16 spr_offset = spr_get_y_offset(ws);
		const i16 y_offset = (i16) ((projection_dist * spr_offset) / ds->distance);
		const angle_d angle = angle_normalize_deg_d(ds->angle - (viewport_x_fov / 2));

		spr_rect.w = (i32) ((projection_dist * ws->w) / ds->distance);
		spr_rect.h = (i32) ((projection_dist * ws->h) / ds->distance);
		spr_rect.x = (i32) (tan(TO_RAD(angle)) * projection_dist) + (viewport_w / 2) - (spr_rect.w / 2);
		spr_rect.y = (i32) (viewport_h / 2) - ((projection_dist * R_HALF_H) / ds->distance) + y_offset;

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
		for (i32 x_i = x_start; x_i < x_end; ++x_i)
		{
			float sprcol_y = spr_rect.y >= 0 ? 0 : -spr_rect.y * y_inc;
			u32* render_px = (u32*) target->pixels + (y_start * viewport_w) + x_i;
			float* z_buffer_px = z_buffer + (y_start * viewport_w) + x_i;
			const u32* const ws_px = (u32*) spr_ptr_begin + (u16) sprcol_x;
			for (i32 y_i = y_start; y_i < y_end; ++y_i)
			{
				u32 spr_px = *(ws_px + (u16) sprcol_y * spritesheet_width);
				if (spr_px != COLOR_KEY && *z_buffer_px > ds->distance)
				{
					const i16 map_x = (i16) floor(ds->actor->x / M_CELLSIZE);
					const i16 map_y = (i16) floor(ds->actor->y / M_CELLSIZE);
					const i16 cell_x = (i16) fmod(ds->actor->x, M_CELLSIZE);
					const i16 cell_y = (i16) fmod(ds->actor->y, M_CELLSIZE);
					spr_px = r_light_px(map_x, map_y, ws->anchor == SPR_CEIL ? M_CEIL : M_FLOOR, spr_px, cell_x, cell_y);
					spr_px = cm_ramp_mix(spr_px, ds->distance);
					*render_px = spr_px;
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


static const spr_frame* spr_get_frame(const spr_actor* a)
{
	const spr_frameset* spr_frames = &SPR_DEF[a->actor->type].frames[a->actor->frame];
	const angle_d range_per_frame = 360. / spr_frames->angles;
	const angle_d relative_angle = angle_normalize_deg_d(range_per_frame / 2 + viewport_angle - a->actor->angle);
	const u8 frame = floor(relative_angle / range_per_frame);
	return &spr_frames->sprites[frame];
}

static bool spr_is_visible(const ac_actor* actor, u32 ds_index)
{
	// Skip the player
	if (actor == ac_get_player())
		return false;

	// 1.If sprite in back 180° skip it, it can't possibly be seen
	// 2.If the sprite is in the view fov, draw it, it's definitely seen
	// 3.Between those two fields, calculate the sprite's width at the projected distance.
	// -- Calculate an error angle for that distance, draw the sprite if it falls in that range
	// -- applying cosine correction makes sprites ungodly large at the edge of the FOV,
	//    so we need to compromise
	const double dx = actor->x - viewport_x;
	const double dy = actor->y - viewport_y;
	const double slope = angle_get_deg_d(dx, -dy);

	const double fov_min_full = angle_normalize_deg_d(viewport_angle - viewport_x_fov);
	const double fov_max_full = angle_normalize_deg_d(viewport_angle + viewport_x_fov);

	// Test if the sprite is outside the player's FOV
	if (fov_min_full > fov_max_full)
	{
		if (slope < fov_min_full && slope > fov_max_full)
			return false;
	}
	else
	{
		if (slope < fov_min_full || slope > fov_max_full)
			return false;
	}

	const double angle = TO_RAD(viewport_angle - slope);
	const double distance = math_dist(viewport_x, viewport_y, actor->x, actor->y);
	const double distance_corrected = distance * cos(angle);

	const double fov_min_half = angle_normalize_deg_d(viewport_angle - (viewport_x_fov / 2));
	const double fov_max_half = angle_normalize_deg_d(viewport_angle + (viewport_x_fov / 2));
	const double angle_to_sprite = angle_normalize_deg_d(fov_max_half - slope);

	// Test if the sprite is inside the player's FOV
	bool drawsprite;
	if (fov_min_half > fov_max_half)
		drawsprite = slope >= fov_min_half || slope <= fov_max_half;
	else
		drawsprite = slope >= fov_min_half && slope <= fov_max_half;

	spr_actor as = { 0 };
	as.actor = actor;
	as.distance = distance_corrected; 
	as.angle = angle_to_sprite;
	if (drawsprite)
	{
		sprite_slot_buffer[ds_index] = as;
		return true;
	}

	// The sprite now partially intersects with the player's view.
	// Todo: Get the projected sprite's width
	spr_frame* ws = spr_get_frame(&as);
	const double spr_width = ((projection_dist * ws->w) / distance_corrected) / 2;
	// Sanity check if the sprite gets way too large
	if (spr_width > viewport_w * 128)
		return false;
	const double px_angle_ratio = (double) viewport_x_fov / viewport_w;
	const double err_angle = px_angle_ratio * spr_width;


	// The error angle now contains the absolute offset that we should look ahead
	const double fov_min_err = angle_normalize_deg_d(fov_min_half - err_angle);
	const double fov_max_err = angle_normalize_deg_d(fov_max_half + err_angle);

	if (fov_min_err > fov_max_err)	//360° rollaround
		drawsprite = slope >= fov_min_err || slope <= fov_max_err;
	else
		drawsprite = slope >= fov_min_err && slope <= fov_max_err;

	if (drawsprite)
	{
		sprite_slot_buffer[ds_index] = as;
		return true;
	}
	return false;
}

static u8 spr_get_y_offset(const spr_frame* ws)
{
	switch (ws->anchor)
	{
	case SPR_CEIL:
		return ws->offset;
	case SPR_CENTER:
		return R_HALF_H - ws->h / 2;
	case SPR_FLOOR:
		return R_CELL_H - ws->h - ws->offset;
	}
}
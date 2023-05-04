#include <render/sprite.h>

#include <map/map.h>
#include <render/colorramp.h>
#include <render/renderconstants.h>
#include <util/mathutil.h>

#include <math.h>

// I'm not sure if we want a set sprite buffer, rather than statically allocating it.
// Then, qsort only works on arrays, and we need some fixed bounds on that.

// Sprite data should probably be externalized to a separate file so data can be edited without
// rebuilding the solution.
#define CORNERS(x1, y1, x2, y2) { (x1), (y1), ((x2) - (x1)), ((y2) - (y1)) }

const WorldSprite worldSprites[] =
{
	{ 0, CORNERS(0, 0, 0, 0), CENTER, 0 },	//Dummy sprite
	{ 0, CORNERS(0, 0, 28, 64), FLOOR, 0 }	//Pillar
};

const ActorFrameSheet frameSheets[] =
{
	{ 1, &worldSprites[0] },
	{ 1, &worldSprites[1] }
};

const ActorSpriteSheet spriteSheets[] =
{
	&frameSheets[0],
	&frameSheets[1]
};

#define MAX_SPRITES 256

ActorSprite sprite_slot_buffer[MAX_SPRITES];

extern Map m_map;
extern ActorList particles, projectiles, tempEnemies;

extern u16 viewport_w, viewport_h, viewport_w_half;

extern float viewport_x, viewport_y, viewport_angle;
extern u8 viewport_x_fov, viewport_x_fov_half;

extern float projection_dist;

extern float* z_buffer;

extern SDL_Surface* gfx_ws_buffer[];

void spr_draw(SDL_Surface* target)
{
	SDL_Rect spr_rect;
	const static ActorList* const actorLists[] =
	{ 
		&m_map.levelPickups, 
		&projectiles, 
		&particles, 
		&tempEnemies 
	};
	const static ActorArray* const actorArrays[] =
	{ 
		&m_map.levelEnemies, 
		&m_map.levelObjs 
	};

	//First off, populate the sprite buffer
	u32 ds_index = 0;
	for (u32 i = 0; i < SDL_arraysize(actorLists); ++i)
		PopulateSpriteBufferList(actorLists[i], &ds_index);
	for (u32 i = 0; i < SDL_arraysize(actorArrays); ++i)
		PopulateSpriteBufferArray(actorArrays[i], &ds_index);

	//Now draw them, back to front
	while (ds_index--)
	{
		const ActorSprite* ds = &sprite_slot_buffer[ds_index];

		// Todo: work out what animation frame and angle to use
		const WorldSprite ws = GetWorldSprite(ds);
		//const u16 max_width = viewport_w + (viewport_w / 4);	//cap the size of the sprite

		const i16 spr_offset = spr_get_y_offset(&ws);
		const i16 y_offset = (i16) ((projection_dist * spr_offset) / ds->distance);
		const angle_d angle = angle_normalize_deg_d(ds->angle - viewport_x_fov_half);

		spr_rect.w = (i32) ((projection_dist * ws.coords.w) / ds->distance);
		//if (spr_rect.w > max_width) spr_rect.w = max_width;
		spr_rect.h = (i32) ((projection_dist * ws.coords.h) / ds->distance);
		spr_rect.x = (i32) (tan(TO_RAD(angle)) * projection_dist) + viewport_w_half - (spr_rect.w / 2);
		spr_rect.y = (i32) (viewport_h / 2) - ((projection_dist * R_HALF_H) / ds->distance) + y_offset;

		// The idea: Make a loop running through all the pixel columns, clipping accordingly at the edges:
		// If and only if the sprite needs to be masked, blit into a specialty area first
		const i32 x_start = SDL_max(spr_rect.x, 0);
		const i32 x_end = SDL_min(spr_rect.x + spr_rect.w, viewport_w);
		const float x_inc = ws.coords.w / (float) spr_rect.w;

		const i32 y_start = SDL_max(spr_rect.y, 0);
		const i32 y_end = SDL_min(spr_rect.y + spr_rect.h, viewport_h);
		const float y_inc = ws.coords.h / (float) spr_rect.h;

		const u16 spritesheet_width = gfx_ws_buffer[ws.spritesheet]->w;
		const u32* spr_ptr_begin = (u32*) gfx_ws_buffer[ws.spritesheet]->pixels + (ws.coords.y * spritesheet_width) + ws.coords.x;

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
					spr_px = r_light_px(map_x, map_y, ws.anchor == CEIL ? M_CEIL : M_FLOOR, spr_px, cell_x, cell_y);
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


static inline WorldSprite GetWorldSprite(const ActorSprite* a)
{
	const u32 spr_i = GetActorSpriteIndex(a->actor->type);
	const ActorSpriteSheet actorSprites = spriteSheets[spr_i];
	const ActorFrameSheet *actorFrames = actorSprites.animation_frames;
	return actorFrames->sprites[0];
}

static inline bool ActorOnScreen(const Actor* actor, u32* ds_index)
{
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

	const double fov_min_half = angle_normalize_deg_d(viewport_angle - viewport_x_fov_half);
	const double fov_max_half = angle_normalize_deg_d(viewport_angle + viewport_x_fov_half);
	const double angle_to_sprite = angle_normalize_deg_d(fov_max_half - slope);

	// Test if the sprite is inside the player's FOV
	bool drawsprite;
	if (fov_min_half > fov_max_half)
		drawsprite = slope >= fov_min_half || slope <= fov_max_half;
	else
		drawsprite = slope >= fov_min_half && slope <= fov_max_half;

	ActorSprite as = { 0 };
	as.actor = actor;
	as.distance = distance_corrected; 
	as.angle = angle_to_sprite;
	if (drawsprite)
	{
		setsprite:
		sprite_slot_buffer[(*ds_index)++] = as;
		return true;
	}

	// The sprite now partially intersects with the player's view.
	// Todo: Get the projected sprite's width
	WorldSprite ws = GetWorldSprite(&as);
	const double spr_width = ((projection_dist * ws.coords.w) / distance_corrected) / 2;
	// Sanity check if the sprite gets way too large
	if (spr_width > viewport_w * 128)
		return false;
	const double px_angle_ratio = (double) viewport_x_fov / viewport_w;
	const double err_angle = px_angle_ratio * spr_width;

	goto setsprite;

	// The error angle now contains the absolute offset that we should look ahead
	const double fov_min_err = angle_normalize_deg_d(fov_min_half - err_angle);
	const double fov_max_err = angle_normalize_deg_d(fov_max_half + err_angle);

	if (fov_min_err > fov_max_err)	//360° rollaround
		drawsprite = slope >= fov_min_err || slope <= fov_max_err;
	else
		drawsprite = slope >= fov_min_err && slope <= fov_max_err;

	if (drawsprite) 
		goto setsprite;
	return false;
}

static void PopulateSpriteBufferList(const ActorList* actors, u32* ds_index)
{
	if (*ds_index >= MAX_SPRITES) 
		return;

	const ActorNode* node = actors->first;
	while (node)
	{
		if (ActorOnScreen(node->content, ds_index))
			if (*ds_index >= MAX_SPRITES) 
				return;
		node = node->next;
	}
}

static void PopulateSpriteBufferArray(const ActorArray* actors, u32 *ds_index)
{
	if (*ds_index >= MAX_SPRITES) 
		return;

	const Actor* act;
	for (u32 i = 0; i < actors->count; ++i)
	{
		act = &actors->actor[i];
		if (ActorOnScreen(act, ds_index))
			if (*ds_index >= MAX_SPRITES) 
				return;
	}
}

static u8 spr_get_y_offset(const WorldSprite* ws)
{
	switch (ws->anchor)
	{
	case CEIL:
		return ws->offset;
	case CENTER:
		return R_HALF_H - ws->coords.h / 2;
	case FLOOR:
		return R_CELL_H - ws->coords.h - ws->offset;
	}
}
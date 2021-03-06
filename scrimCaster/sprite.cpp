#include "types.h"
#include "sprite.h"
#include "map.h"
#include <math.h>
#include "SDL/SDL_assert.h"
#include "renderconstants.h"
#include "SDL/SDL_log.h"

//I'm not sure if we want a set sprite buffer, rather than statically allocating it.
//Then, qsort only works on arrays, and we need some fixed bounds on that.

//Sprite data should probably be externalized to a separate file so data can be edited without
//rebuilding the solution.
#define CORNERS(x1,y1,x2,y2) { (x1), (y1), ((x2) - (x1)), ((y2) - (y1)) }

const WorldSprite worldSprites[] =
{
	{ 0, CORNERS(0, 0, 0, 0), CENTER, 0 },	//Dummy sprite
	{ 0, CORNERS(0, 0, 28, 64), FLOORCEIL, 0 }	//Pillar
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

const u16 MAX_SPRITES = 256;

const float SPRITE_RATIO_END = 8e-4f;

//SDL_Surface* sprite_buffer = NULL;
u32* sprite_buffer = NULL;
ActorSprite sprite_slot_buffer[MAX_SPRITES];

extern Map map;
extern ActorList particles, projectiles, tempEnemies;

extern u16 viewport_w, viewport_h, viewport_w_half;

extern float viewport_x, viewport_y, viewport_angle;
extern Map map;
extern u8 viewport_x_fov, viewport_x_fov_half;

extern float projection_dist;

extern float* z_buffer[];
extern SDL_Surface* blend_masks[];
extern u8* layer_count;

extern SDL_Surface* worldSpriteBuffer[];

void DrawSprites(SDL_Surface* toDraw)
{
	SDL_Rect spr_rect;
	const static ActorList* const actorLists[] =
	{ &map.levelPickups, &projectiles, &particles, &tempEnemies };
	const static ActorArray* const actorArrays[] =
	{ &map.levelEnemies, &map.levelObjs };

	//First off, populate the sprite buffer
	u32 ds_index = 0, i;
	for (i = 0; i < SDL_arraysize(actorLists); ++i)
		PopulateSpriteBufferList(actorLists[i], &ds_index);
	for (i = 0; i < SDL_arraysize(actorArrays); ++i)
		PopulateSpriteBufferArray(actorArrays[i], &ds_index);
	SDL_qsort(sprite_slot_buffer, ds_index, sizeof(ActorSprite), SpriteDistSort);

	//Now draw them, back to front
	while (ds_index--)
	{
		const ActorSprite* ds = &sprite_slot_buffer[ds_index];

		//Todo: work out what animation frame and angle to use
		WorldSprite ws = GetWorldSprite(ds);
		const i32 height = ws.anchor == FLOORCEIL ? CELLHEIGHT : ws.coords.h;
		//const u16 max_width = viewport_w + (viewport_w / 4);	//cap the size of the sprite

		const i16 h_offset = ws.anchor == FLOORCEIL ? 0 :
			i16((projection_dist * ws.offset +
			((CELLHEIGHT / 2) + (ws.coords.h / 2) * ws.anchor == FLOOR ? 1 : -1) / ds->dist));

		const double angle = ds->relative_angle - 
			(ds->relative_angle > 180 ? 360 : 0)
			- viewport_x_fov_half;

		spr_rect.w = i32((projection_dist * ws.coords.w) / ds->dist);
		//if (spr_rect.w > max_width) spr_rect.w = max_width;
		spr_rect.h = i32((projection_dist * height) / ds->dist);
		spr_rect.x = i32(tan(TO_RAD(angle))*projection_dist) + (viewport_w_half) - (spr_rect.w / 2);
		spr_rect.y = h_offset + ((viewport_h - spr_rect.h) / 2);

		//Shouldn't happen - todo, investigate what's going wrong
		if (spr_rect.x >= viewport_w || spr_rect.y >= viewport_h) continue;
		if (spr_rect.x + spr_rect.w < 0 || spr_rect.y + spr_rect.h < 0) continue;

		//The idea: Make a loop running through all the pixel collumns, clipping accordingly at the edges:
		//If and only if the sprite needs to be masked, blit into a specialty area first
		const i32 x_start = SDL_max(spr_rect.x, 0);
		const i32 x_end = SDL_min(spr_rect.x + spr_rect.w, viewport_w);
		const float x_inc = ws.coords.w / float(spr_rect.w) - SPRITE_RATIO_END;

		const i32 y_start = SDL_max(spr_rect.y, 0);
		const i32 y_end = SDL_min(spr_rect.y + spr_rect.h, viewport_h);
		const i32 y_diff = y_end - y_start;
		const float y_inc = ws.coords.h / float(spr_rect.h) - SPRITE_RATIO_END;

		const u16 spritesheet_width = worldSpriteBuffer[ws.spritesheet]->w;
		const u32* spr_ptr_begin = (u32*)worldSpriteBuffer[ws.spritesheet]->pixels +
			(ws.coords.y * spritesheet_width) + ws.coords.x;

		float sprcol_x = spr_rect.x >= 0 ? 0 : -spr_rect.x * x_inc;
		for (i32 x_i = x_start; x_i < x_end; ++x_i)
		{
			if (ds->dist > z_buffer[0][x_i])
				goto skip;

			float sprcol_y = spr_rect.y >= 0 ? 0 : -spr_rect.y * y_inc;
			const u8 layercount = layer_count[x_i];
			u32* render_px = (u32*)toDraw->pixels + (y_start * viewport_w) + x_i;
			const u32* const ws_px = (u32*)spr_ptr_begin + (u16)sprcol_x;

			if (layercount && ds->dist > z_buffer[1][x_i])		//We need to mask something
			{
				u8 l = 0;
				u32* const spr_buf_px = sprite_buffer;
				
				for (i32 y_buf = 0; y_buf < y_diff; ++y_buf)	//Write sprite into temporary buffer for masking
				{
					spr_buf_px[y_buf] = *(ws_px + (u16)sprcol_y * spritesheet_width);
					sprcol_y += y_inc;
				}
				while (l < layercount && ds->dist > z_buffer[l+1][x_i])	//Mask
				{
					u32* blend_px = (u32*)blend_masks[l++]->pixels + (y_start * viewport_w) + x_i;
					for (i32 y_buf = 0; y_buf < y_diff; ++y_buf)
						if (blend_px[y_buf * viewport_w]) spr_buf_px[y_buf] = COLOR_KEY;
				}
				for (i32 y_buf = 0; y_buf < y_diff; ++y_buf)	//Draw masked sprite in world
				{
					const u32 spr_col = spr_buf_px[y_buf];
					if (spr_col != COLOR_KEY)
						*render_px = spr_col;
					render_px += viewport_w;
				}
			}
			else for (i32 y_i = y_start; y_i < y_end; ++y_i)	//Draw the sprite without masking
			{
				const u32 spr_col = *(ws_px + (u16)sprcol_y * spritesheet_width);
				if (spr_col != COLOR_KEY)
					*render_px = spr_col;
				sprcol_y += y_inc;
				render_px += viewport_w;
			}
		skip:
			sprcol_x += x_inc;
		}
	}
}


static inline WorldSprite GetWorldSprite(const ActorSprite* a)
{
	const u32 spr_i = GetActorSpriteIndex(a->actor);
	const ActorSpriteSheet actorSprites = spriteSheets[spr_i];
	const ActorFrameSheet *actorFrames = actorSprites.animation_frames;
	return actorFrames->sprites[0];
}

static inline bool ActorOnScreen(const Actor* actor, u32 *ds_index)
{
	//1.If sprite in back 180� skip it, it can't possibly be seen
	//2.If the sprite is in the view fov, draw it, it's definitely seen
	//3.Between those two fields, calculate the sprite's width at the projected distance.
	//--Calculate an error angle for that distance, draw the sprite if it falls in that range
	//-- applying cosine correction makes sprites ungodly large at the edge of the FOV,
	//-- so we need to compromise

	const double x_disp = actor->x - viewport_x;
	const double y_disp = actor->y - viewport_y;
	const double slope = AngleToDeg(y_disp * -1, x_disp);

	const double fov_min_full = SubtractAngle(viewport_angle, viewport_x_fov);
	const double fov_max_full = AddAngle(viewport_angle, viewport_x_fov);

	bool skipsprite;
	if (fov_min_full > fov_max_full)
		skipsprite = slope < fov_min_full && slope > fov_max_full;
	else
		skipsprite = slope < fov_min_full || slope > fov_max_full;
	if (skipsprite) return false;

	const double relative_angle = TO_RAD(viewport_angle - slope);
	const double cos_ra = cos(relative_angle);
	const double spr_dist = sqrt(pow(x_disp, 2) + pow(y_disp, 2)) * cos_ra;

	const double fov_min_half = SubtractAngle(viewport_angle, viewport_x_fov_half);
	const double fov_max_half = AddAngle(viewport_angle, viewport_x_fov_half);
	const double angle_to_sprite = SubtractAngle(fov_max_half, slope);

	bool drawsprite;
	if (fov_min_half > fov_max_half)	//360� rollaround
		drawsprite = slope >= fov_min_half || slope <= fov_max_half;
	else
		drawsprite = slope >= fov_min_half && slope <= fov_max_half;

	const ActorSprite as = { actor, spr_dist, angle_to_sprite };
	if (drawsprite)
	{
		setsprite:
		sprite_slot_buffer[(*ds_index)++] = as;
		return true;
	}

	//Todo: Get the projected sprite's width
	WorldSprite ws = GetWorldSprite(&as);
	double spr_width = ((projection_dist * ws.coords.w) / spr_dist) / 2;
	const double px_angle_ratio = (double)viewport_x_fov / viewport_w;
	const double err_angle = px_angle_ratio * spr_width;
	//The error angle now contains the absolute offset that we should look ahead

	const double fov_min_err = SubtractAngle(fov_min_half, err_angle);
	const double fov_max_err = AddAngle(fov_max_half, err_angle);

	if (fov_min_err > fov_max_err)	//360� rollaround
		drawsprite = slope >= fov_min_err || slope <= fov_max_err;
	else
		drawsprite = slope >= fov_min_err && slope <= fov_max_err;

	if (drawsprite) goto setsprite;
	return false;
}

static void PopulateSpriteBufferList(const ActorList* actors, u32 *ds_index)
{
	if (*ds_index >= MAX_SPRITES) return;

	const ActorNode* node = actors->first;
	while (node)
	{
		if (ActorOnScreen(node->content, ds_index))
			if (*ds_index >= MAX_SPRITES) return;
		node = node->next;
	}
}

static void PopulateSpriteBufferArray(const ActorArray* actors, u32 *ds_index)
{
	if (*ds_index >= MAX_SPRITES) return;

	const Actor* act;
	for (u32 i = 0; i < actors->count; ++i)
	{
		act = &actors->actor[i];
		if (ActorOnScreen(act, ds_index))
			if (*ds_index >= MAX_SPRITES) return;
	}
}

static inline double SubtractAngle(double a, double b)
{
	return NormalizeAngleUp(a - b);
}

static inline double AddAngle(double a, double b)
{
	return NormalizeAngleDown(a + b);
}

static inline double SubtractAngle(double a, u8 b)
{
	return NormalizeAngleUp(a - b);
}

static inline double AddAngle(double a, u8 b)
{
	return NormalizeAngleDown(a + b);
}

static inline double NormalizeAngleUp(double a)
{
	while (a < 0)
		a += 360;
	return a;
}

static inline double NormalizeAngleDown(double a)
{
	while (a >= 360)
		a -= 360;
	return a;
}

static i32 SpriteDistSort(const void* p1, const void* p2)
{
	const ActorSprite* const d1 = (ActorSprite*)p1;
	const ActorSprite* const d2 = (ActorSprite*)p2;

	if (d1->dist > d2->dist) return  1;
	if (d1->dist < d2->dist) return -1;
	return 0;
}
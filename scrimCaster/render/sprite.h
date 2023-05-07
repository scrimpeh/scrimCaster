#pragma once

#include <common.h>

#include <game/actor/actor.h>
#include <game/actor/actorcontainers.h>

#include <SDL/SDL_surface.h>

#define MAXWIDTH 256 // Maximum width of a sprite in the sheet

// An actor on screen
typedef struct
{
	const ac_actor* actor;
	double distance;
	double angle;
} spr_actor;

typedef enum
{
	SPR_CENTER = 0,
	SPR_FLOOR  = 1,
	SPR_CEIL   = 2
} spr_anchor;

// Static sprite definition for one sprite
typedef struct
{
	// The sprite sheet index to use
	u16 sheet;
	// The coordinates on the sprite sheet
	u16 x;
	u16 y;
	u16 w;
	u16 h;
	// The anchor
	spr_anchor anchor;
	// How many pixels to draw the sprite from the anchor (applicable only to floor or ceil)
	i16 offset;
} spr_frame;

// Bundles a set of spr_frames from several angle
typedef struct
{
	u8 angles;
	const spr_frame* sprites;
} spr_frameset;

// Maps all animation frames of an actor to a frameset
typedef struct
{
	const spr_frameset* frames;
} spr_actor_frame_map;

static bool spr_is_visible(const ac_actor* actor, u32 ds_index);
static const spr_frame* spr_get_frame(const spr_actor* a);

void spr_draw(SDL_Surface* target);

static u8 spr_get_y_offset(const spr_frame* ws);
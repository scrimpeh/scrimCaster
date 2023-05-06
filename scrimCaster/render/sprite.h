#pragma once

#include <common.h>

#include <game/actor/actor.h>
#include <game/actor/actorcontainers.h>

#include <SDL/SDL_surface.h>

#define MAXWIDTH 256 // Maximum width of a sprite in the sheet

typedef struct
{
	const Actor* actor;
	double distance;
	double angle;
} ActorSprite;

typedef enum
{
	CENTER = 0,
	FLOOR = 1,
	CEIL = 2
} Anchor;

// Defines a sprite as to draw on screen, static struct, should only be used
// for data definition
typedef struct
{
	// The sprite sheet index to use
	u16 spritesheet;
	// The coordinates on the sprite sheet
	SDL_Rect coords;
	// The anchor
	Anchor anchor;
	// How many pixels to draw the sprite from the anchor (applicable only to floor or ceil)
	i16 offset;
} WorldSprite;

typedef struct
{
	u8 angles;
	const WorldSprite *sprites;
} ActorFrameSheet;

typedef struct
{
	const ActorFrameSheet *animation_frames;
} ActorSpriteSheet;

static inline bool ActorOnScreen(const Actor* actor, u32* ds_index);
static void PopulateSpriteBufferList(const ActorList* actors, u32* ds_index);
static void PopulateSpriteBufferArray(const ActorArray* actors, u32* ds_index);
static inline WorldSprite GetWorldSprite(const ActorSprite* a);

void spr_draw(SDL_Surface* target);

static u8 spr_get_y_offset(const WorldSprite* ws);
#pragma once

#include "common.h"

#include "SDL/SDL_surface.h"
#include "actor.h"
#include "actorcontainers.h"

#define MAXWIDTH 256 // Maximum width of a sprite in the sheet

typedef struct ActorSprite
{
	const Actor* actor;
	double dist;
	double relative_angle;
} ActorSprite;

typedef enum Anchor : u8
{
	CENTER = 0,
	FLOOR = 1,
	CEIL = 2,
	FLOORCEIL = 3
} Anchor;

// Defines a sprite as to draw on screen, static struct, should only be used
// for data definition
typedef struct WorldSprite
{
	u16 spritesheet;
	SDL_Rect coords;	//The coordinates on the sprite sheet
	Anchor anchor;
	i16 offset;
} WorldSprite;

typedef struct ActorFrameSheet
{
	u8 angles;
	const WorldSprite *sprites;
} ActorFrameSheet;

typedef struct ActorSpriteSheet
{
	const ActorFrameSheet *animation_frames;
} ActorSpriteSheet;

static inline bool ActorOnScreen(const Actor* actor, u32 *ds_index);
static void PopulateSpriteBufferList(const ActorList* actors, u32 *ds_index);
static void PopulateSpriteBufferArray(const ActorArray* actors, u32 *ds_index);

void DrawSprites(SDL_Surface* toDraw);
static i32 SpriteDistSort(const void* p1, const void* p2);

static inline WorldSprite GetWorldSprite(const ActorSprite* a);

static inline double SubtractAngle(double a, double b);
static inline double AddAngle(double a, double b);
static inline double SubtractAngle(double a, u8 b);
static inline double AddAngle(double a, u8 b);

static inline double NormalizeAngleUp(double a);
static inline double NormalizeAngleDown(double a);
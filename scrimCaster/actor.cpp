#include "types.h"
#include "actor.h"
#include "actorcontainers.h"
#include "map.h"
#include "renderconstants.h"

#include "SDL/SDL_log.h"
#include "SDL/SDL_assert.h"

#define WALL_COLLISION 1
#define ACTOR_COLLISION 2

const double MIN_WALL_DIST = 1e-12;

Bounds curBounds;

extern Map map;
extern ActorList tempEnemies, projectiles, particles;
extern ActorVector levelEnemies;

//This should presumably be offloaded to a separate file, either a source file 
//or perhaps even an external file
const Bounds NullBounds = { 0., 0. };
const Bounds PlayerBounds = { (double)CELLSIZE / 8, (double)CELLSIZE / 8 };
const Bounds PillarBounds = PlayerBounds;

const static ActorList* const actorLists[] =
	{ /*&currentMap.levelPickups, &projectiles, &particles,*/ &tempEnemies };
const static ActorArray* const actorArrays[] =
	{ &map.levelEnemies, &map.levelObjs };
const static ActorVector* const actorVectors[] =
	{ &levelEnemies };

Bounds GetActorBounds(const Actor* actor)
{
	return GetActorBounds(actor->type);
}

Bounds GetActorBounds(ActorType type)
{
	const u16 upper = type & 0xFF00;
	const u8 lower = type & 0xFF;

	switch (upper)
	{
	case ENEMY:
		return GetEnemyBounds(lower);
	case PROJ:
		return GetProjBounds(lower);
	case LEVELOBJ:
		return GetLevelObjBounds(lower);
	case PICKUP:
		return GetPickupBounds(lower);
	case OTHER:
		return GetOtherBounds(lower);
	default:
		switch (lower)
		{
		case PLAYER:
			return PlayerBounds;
		default:
			return { 0., 0. };
		}
	}
}

//Transforms an actor index into an offset for sprite arrays and such
u32 GetActorSpriteIndex(const Actor* actor)
{
	return GetActorSpriteIndex(actor->type);
}

u32 GetEnemySpriteIndex(u8 type)
{
	return 0;
}

u32 GetProjSpriteIndex(u8 type)
{
	return 0;
}

u32 GetLevelObjSpriteIndex(u8 type)
{
	static const u32 types[] = { 0, 1 };
	return types[type];
}

u32 GetPickupSpriteIndex(u8 type)
{
	return 0;
}

u32 GetParticleSpriteIndex(u8 type)
{
	return 0;
}

u32 GetOtherSpriteIndex(u8 type)
{
	return 0;
}

u32 GetActorSpriteIndex(ActorType type)
{
	const u16 upper = type & 0xFF00;
	const u8 lower = type & 0xFF;

	switch (upper)
	{
	case ENEMY:
		return GetEnemySpriteIndex(lower);
	case PROJ:
		return GetProjSpriteIndex(lower);
	case LEVELOBJ:
		return GetLevelObjSpriteIndex(lower);
	case PICKUP:
		return GetPickupSpriteIndex(lower);
	case PARTICLE:
		return GetParticleSpriteIndex(lower);
	case 0:
	case OTHER:
	default:
		return GetOtherSpriteIndex(lower);
	}
}

Bounds GetEnemyBounds(u8 type)
{
	static const Bounds bounds[] = { NullBounds, PillarBounds };
	return bounds[type];
}

Bounds GetProjBounds(u8 type)
{
	return{ 0, 0 };
}

Bounds GetLevelObjBounds(u8 type)
{
	static const Bounds bounds[] = { NullBounds, PillarBounds };
	return bounds[type];
}

Bounds GetPickupBounds(u8 type)
{
	return { 0, 0 };
}

Bounds GetOtherBounds(u8 type)
{
	return { 0, 0 };
}


static bool CollideHorizontalWall(const Actor* actor, double* p_dx)
{
	double d_x = *p_dx;

	if (d_x == 0) return false;

	const double bounds_x = curBounds.x * (d_x >= 0 ? 1 : -1);
	const i16 x_start = (i16)SDL_floor((actor->x - bounds_x) / CELLSIZE);					 //Draw a line on the actor's movement plane
	const i16 x_end = (i16)SDL_floor((actor->x + d_x + bounds_x) / CELLSIZE); //and check if he hit a wall
	const i16 y_top = (i16)SDL_floor((actor->y - curBounds.y) / CELLSIZE);
	const i16 y_bottom = (i16)SDL_floor((actor->y + curBounds.y) / CELLSIZE);

	const Cell* cells = map.cells;

	const i8 inc = x_start > x_end ? -1 : 1;
	for (i16 x = x_start; x != x_end; x += inc)
	{
		//Todo: This might be optimized by just adding to pointers directly.
		const Cell cell_top = cells[y_top * map.w + x];
		const Cell cell_bottom = cells[y_bottom * map.w + x];

		const Side side_top = d_x > 0 ? cell_top.e : cell_top.w;
		const Side side_bottom = d_x > 0 ? cell_bottom.e : cell_bottom.w;

		if ((side_top.type && !(side_top.flags  & PASSABLE)) ||
			(side_bottom.type && !(side_bottom.flags & PASSABLE)))
		{
			double new_x = double(x*CELLSIZE) - bounds_x;
			if (d_x >= 0) new_x += double(CELLSIZE) - MIN_WALL_DIST;
			*p_dx = new_x - actor->x;
			return true;
		}
	}

	return false;
}

static bool CollideVerticalWall(const Actor* actor, double* p_dy)
{
	double d_y = *p_dy;

	if (d_y == 0) return false;

	const double bounds_y = curBounds.y * (d_y >= 0 ? 1 : -1);
	const i16 y_start = (i16)SDL_floor((actor->y - bounds_y) / CELLSIZE);					 //Draw a line on the actor's movement plane
	const i16 y_end = (i16)SDL_floor((actor->y + d_y + bounds_y) / CELLSIZE); //and check if he hit a wall
	const i16 x_left = (i16)SDL_floor((actor->x - curBounds.x) / CELLSIZE);
	const i16 x_right = (i16)SDL_floor((actor->x + curBounds.x) / CELLSIZE);

	const Cell* cells = map.cells;

	const i8 inc = y_start > y_end ? -1 : 1;
	for (i16 y = y_start; y != y_end; y += inc)
	{
		const Cell cell_left = cells[y * map.w + x_left];
		const Cell cell_right = cells[y * map.w + x_right];

		const Side side_left = d_y > 0 ? cell_left.s : cell_left.n;
		const Side side_right = d_y > 0 ? cell_right.s : cell_right.n;

		if ((side_left.type  && !(side_left.flags  & PASSABLE)) ||
			(side_right.type && !(side_right.flags & PASSABLE)))
		{
			double new_y = double(y*CELLSIZE) - bounds_y;
			if (d_y >= 0) new_y += double(CELLSIZE) - MIN_WALL_DIST;
			*p_dy = new_y - actor->y;
			return true;
		}
	}

	return false;
}

static bool IntersectActor(Actor* mover, const Actor* obstacle, bool vertical, double* disp)
{
	//perhaps split up the actor to actor collision in two methods for horizontal
	//and vertical
	if (mover == obstacle) return false;

	Bounds obstacleBounds = GetActorBounds(obstacle->type);

	const double d_xy = *disp;
	double a_right, a_top, a_left, a_bottom;
	double b_right, b_top, b_left, b_bottom;

	if (vertical)
	{
		a_right =  mover->x        + curBounds.x;
		a_left =   mover->x        - curBounds.x;
		a_bottom = mover->y + d_xy + curBounds.y;
		a_top =    mover->y + d_xy - curBounds.y;
	}
	else
	{
		a_right =  mover->x + d_xy + curBounds.x;
		a_left =   mover->x + d_xy - curBounds.x;
		a_bottom = mover->y        + curBounds.y;
		a_top =    mover->y        - curBounds.y;
	}

	b_right  = obstacle->x + obstacleBounds.x;
	b_left   = obstacle->x - obstacleBounds.x;
	b_bottom = obstacle->y + obstacleBounds.y;
	b_top    = obstacle->y - obstacleBounds.y;

	if (a_right < b_left || a_left > b_right ||
		a_bottom < b_top || a_top > b_bottom)
		return false;

	double new_pos;
	if (vertical)
	{
		if (d_xy >= 0)
			new_pos = obstacle->y - obstacleBounds.y - curBounds.y - MIN_WALL_DIST;
		else
			new_pos = obstacle->y + obstacleBounds.y + curBounds.y + MIN_WALL_DIST;
		*disp = new_pos - mover->y;
	}
	else
	{
		if (d_xy >= 0)
			new_pos = obstacle->x - obstacleBounds.x - curBounds.x - MIN_WALL_DIST;
		else
			new_pos = obstacle->x + obstacleBounds.x + curBounds.x + MIN_WALL_DIST;
		*disp = new_pos - mover->x;
	}

	return true;
}

static bool CollideActor(Actor* actor, bool vertical, double* disp)
{
	u8 i;
	bool collision = false;

	//Run through all the actors in the list
	for (i = 0; i < SDL_arraysize(actorArrays); ++i)
	{
		const ActorArray* arr = actorArrays[i];
		for (u32 j = 0; j < arr->count; ++j)
		{
			const Actor* const a = arr->actor + j;
			collision |= IntersectActor(actor, a, vertical, disp);
		}
	}

	for (i = 0; i < SDL_arraysize(actorVectors); ++i)
	{
		const ActorVector* av = actorVectors[i];
		for (u32 j = 0; j < av->count; ++j)
		{
			const Actor* const a = av->content[j];
			collision |= IntersectActor(actor, a, vertical, disp);
		}
	}

	for (i = 0; i < SDL_arraysize(actorLists); ++i)
	{
		const ActorNode* an = actorLists[i]->first;
		while (an)
		{
			const Actor* const a = an->content;
			collision |= IntersectActor(actor, a, vertical, disp);
			an = an->next;
		}
	}

	return collision;
}

//returns whether a collision has occured
bool MoveActor(Actor* actor, u32 delta, u32 flags)
{
	bool collision = false;
	double arctan, d_x, d_y;
	arctan = TO_RAD(ATAN_DEG(actor->angle));

	const double cos_atan = SDL_cos(arctan);
	const double sin_atan = SDL_sin(arctan);

	d_x = actor->speed * cos_atan;		//Calculate forward momentum
	d_y = actor->speed * sin_atan * -1;
	d_x += actor->strafe * sin_atan * -1;	//Add strafing momentum
	d_y += actor->strafe * cos_atan * -1;

	//Now adjust the displacement for collision
	curBounds = GetActorBounds(actor->type);

		if (flags & ACTOR_COLLISION)
		collision |= CollideActor(actor, true, &d_y);
	if (flags & WALL_COLLISION)
		collision |= CollideVerticalWall(actor, &d_y);
	actor->y += d_y;

	if (flags & ACTOR_COLLISION)
		collision |= CollideActor(actor, false, &d_x);
	if (flags & WALL_COLLISION)
		collision |= CollideHorizontalWall(actor, &d_x);
	actor->x += d_x;

	return collision;
}

//Could be replaced by a call to fmod
void ActorNormalizeAngle(Actor* actor)
{
	double a = actor->angle;
	while (a < 0)
		a += 360;
	while (a >= 360)
		a -= 360;
	actor->angle = a;
}
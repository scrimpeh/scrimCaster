#include "types.h"
#include "mapupdate.h"

#include "SDL/SDL_assert.h"
#include "SDL/SDL_log.h"

#define DOOR_SCROLL_MAX (M_CELLHEIGHT - 4)
#define DOOR_TICKS_PER_SEC 60
#define PLAYER_HEIGHT 32

#define DOOR_OPEN (TRANSLUCENT)
#define DOOR_PASSABLE (BULLETS_PASS | PASSABLE)
#define DOOR_CLOSED ~DOOR_OPEN
#define DOOR_IMPASSABLE ~DOOR_PASSABLE

SideList* activeSides = NULL;
extern Map map;

void UpdateSides(u32 timeStep)
{
	//This is where things like doors and switches are set. It would be wasteful to run through 
	//the entire map each frame, so we just add it to the list.
	SideList *curSide = activeSides, *lastSide = NULL, *toFree = NULL;
	while (curSide)
	{
		Side* s = curSide->side;
		//Now work out what side it is
		if (s->flags & DOOR_H || s->flags & DOOR_V)
			if (UpdateDoor(s, timeStep)) goto remove_side;

		lastSide = curSide;
		curSide = curSide->next;
		continue;

	remove_side:
		toFree = curSide;
		curSide = curSide->next;

		if (lastSide)
			lastSide->next = curSide;
		else
			activeSides = curSide;

		SDL_free(toFree);
	}
};

SideList* AddActiveSide(Side* s)
{
	SideList* curSide = activeSides, *prevSide = NULL;
	while (curSide)
	{
		if (curSide->side == s) return curSide;

		prevSide = curSide;
		curSide = curSide->next;
	}

	SideList* newSide = (SideList*)SDL_malloc(sizeof(SideList));

	if (newSide)
	{
		newSide->side = s;
		newSide->next = NULL;
	}
	
	if (!activeSides)
		activeSides = newSide;
	else
		prevSide->next = newSide;

	return newSide;
}

void RemoveActiveSide(SideList* curr, SideList* prev)
{
	prev ? prev->next : activeSides = curr->next;	//ternary lvalues woo!
	SDL_free(curr);
}

void ClearActiveSides()
{
	SideList *curSide = activeSides, *delSide = NULL;
	while (curSide)
	{
		delSide = curSide;
		curSide = curSide->next;
		SDL_free(delSide);
	}
}

//Update routines for individual sides
static bool UpdateDoor(Side* const side, u32 timeStep)
{
	//Todo: Change this to determine passability to actors
	//by individual actors and their height, not by the door
	// i.e. move code that determines if this door is pasasble
	// to actor code

	DoorParams* const params = &side->door;
	const u8 ticks = params->timer_ticks + timeStep;
	bool retval = false;

	switch (params->status & 3)
	{
	default:
		SDL_assert(0);
		break;
	case 0:		//activated
		side->flags = SideFlags(side->flags | DOOR_OPEN);
		params->timer_ticks = 0;
		++params->status;
		break;
	case 1:		//opening, below player height
		params->scroll += ticks / params->openspeed;
		params->timer_ticks = ticks % params->openspeed;

		if (params->scroll > PLAYER_HEIGHT)
			side->flags = SideFlags(side->flags | DOOR_PASSABLE);

		if (params->scroll > DOOR_SCROLL_MAX - 1)
		{
			if (params->staytime)
			{
				params->scroll = DOOR_SCROLL_MAX;
				params->timer_ticks = 0;
				params->timer_staycounter = params->staytime * DOOR_TICKS_PER_SEC;
				++params->status;
			}
			else retval = true;;
		}
		break;
	case 2:		//stay open
		params->timer_staycounter -= timeStep;

		if (params->timer_staycounter <= 0)
			++params->status;

		break;
	case 3:
		params->scroll -= ticks / params->closespeed;
		params->timer_ticks = ticks % params->closespeed;

		if (params->scroll < PLAYER_HEIGHT)
			side->flags = SideFlags(side->flags & DOOR_IMPASSABLE);

		if (params->scroll <= 0)
		{
			side->flags = SideFlags(side->flags & DOOR_CLOSED);
			params->scroll = 0;
			params->status = 0;
			retval = true;
		}
		break;
	}

	if (params->door_flags & LINKED)
	{
		Side* const other_door = FromMapOffset(params->linked_to);

		SDL_assert(other_door->type == 5);

		SDL_assert(other_door);

		other_door->door.scroll = params->scroll;
		other_door->door.status = params->status;
		other_door->flags = side->flags;
	}

	return retval;
}
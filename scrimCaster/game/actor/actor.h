#pragma once

#include <common.h>

// On actor types
/* Actor types:
               | Coords | Movement | Angle	| HP   | Status flags/Timers | Other stuff
   Player      | Yes    | Yes	   | Yes	| No** | No					 | -**
   Enemy	   | Yes    | Yes	   | Yes	| Yes  | Yes				 | 
   LevelObject | Yes    | P. No	   | Yes	| ***  | Destroyed/Not D.	 |
   Pickup	   | Yes    | P. No    | P. No	| ***  | Collected/Not C.	 |
   Projectiles | Yes    | Yes	   | Yes	| No.  | No					 |
   Particles   | Yes    | P. No    | P. No	| No.  | P. No				 |
  
   * P.No = Probably no, may not hurt to support regardless
   ** Can use unique variables in player.cpp instead
   *** If I want those objects to be breakable
 */

typedef struct Bounds
{
	double x, y;
} Bounds;

//Actor types are named by some flag, and then an offset
typedef enum TypeOffset
{
	NONE     = 0x0000,
	ENEMY    = 0x0100,
	PROJ     = 0x0200,
	LEVELOBJ = 0x0400,
	PICKUP   = 0x0800,
	PARTICLE = 0x1000,
	OTHER    = 0x2000
} TypeOffset;

typedef enum ActorType
{
	BLANK = 0,
	PLAYER = 1,

	DUMMY_ENEMY = ENEMY | 1,

	PILLAR = LEVELOBJ | 1
} ActorType;

//I'm thinking I should just use Actors for everything, including small stuff.
typedef struct Actor
{
	//Universal types, common for all actors
	ActorType type;
	double x, y; //The position of an actor is given as cartesian coordinates.
	double speed, strafe; //momentum, forward and backward
	angle_d angle;

	//Extra variables, while the names may be indicative of the purpose,
	//they can be used for whatever makes sense for the type
	u16 hp;
	u8 animation_frame;
	u8 state;
	u8 timer;
	u32 flags;
} Actor;

u32 GetActorSpriteIndex(ActorType type);

u32 GetEnemySpriteIndex(u8 type);
u32 GetProjSpriteIndex(u8 type);
u32 GetLevelObjSpriteIndex(u8 type);
u32 GetPickupSpriteIndex(u8 type);
u32 GetParticleSpriteIndex(u8 type);
u32 GetOtherSpriteIndex(u8 type);

Bounds GetActorBounds(ActorType type);

Bounds GetEnemyBounds(u8 type);
Bounds GetProjBounds(u8 type);
Bounds GetLevelObjBounds(u8 type);
Bounds GetPickupBounds(u8 type);
Bounds GetOtherBounds(u8 type);

bool MoveActor(Actor* actor, u32 delta, u32 flags);


static bool CollideHorizontalWall(const Actor* actor, double* p_dx);
static bool CollideVerticalWall(const Actor* actor, double* p_dy);
static bool CollideActor(Actor* actor, bool vertical, double* disp);
static bool IntersectActor(Actor* mover, const Actor* obstacle, bool vertical, double* disp);
#pragma once

#include <common.h>

#include <map/cell.h>

typedef struct
{
	double w;
	double h;
} ac_bounds;

#define AC_BOUNDS_MAX M_CELLSIZE

typedef enum
{
	AC_DUMMY,
	AC_PLAYER,
	AC_DUMMY_ENEMY,
	AC_PILLAR,
	AC_T_LIGHT_FLICKER
} ac_type;

typedef enum
{
	AC_FLAG_SOLID = 0x1
} ac_flags;

typedef struct
{
	// Universal types, common for all actors
	ac_type type;
	double x;
	double y;
	double speed;
	double strafe;
	angle_d angle;
	u8 frame;

	// Extra variables, while the names may be indicative of the purpose,
	// they can be used for whatever makes sense for the type
	u16 hp;
	u8 state;
	i16 timer;
	u32 flags;
} ac_actor;

#define AC_MOVE_COLLIDE_WALL  1
#define AC_MOVE_COLLIDE_ACTOR 2

ac_bounds ac_get_bounds(ac_type type);
bool ac_move(ac_actor* actor, u32 delta, u32 flags);

static bool ac_collide_h(const ac_actor* actor, double* p_dx);
static bool ac_collide_v(const ac_actor* actor, double* p_dy);
static bool ac_collide_actor(ac_actor* actor, bool vertical, double* disp);
static bool ac_intersect(ac_actor* mover, const ac_actor* obstacle, bool vertical, double* disp);
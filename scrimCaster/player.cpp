#include "types.h"
#include "player.h"
#include "actor.h"
#include "input.h"
#include "map.h"
#include "maputil.h"
#include "mapupdate.h"
#include "float.h"

#include "SDL/SDL_assert.h"
#include "SDL/SDL_log.h"

extern Input input, input_tf;
Actor player;

bool noclip = false;

double accel_forward = 0, accel_strafe = 0;
const double TURNSPEED = .2;
const double MAXSPEED = .38, MAXSTRAFE = .35;

const double USELENGTH = (CELLSIZE / 2);

void PlayerStartMap()
{
	player.type = PLAYER;
	player.x = 50;
	player.y = 50;
	player.angle = 0;
}

static void Use()
{
	const Coordinates p_c = { player.x, player.y };
	const Coordinates c = ProjectVector(p_c, player.angle, USELENGTH);
	SideCoords s;

	if( IntersectWall(&s, p_c, c) )
	{
		if (s.side->flags & DOOR_V)
		{
			if ((s.side->door.status & 3) == 0)
			{
				s.side->door.status = 0;
				AddActiveSide(s.side);
			}
		}
	}
}

static void Fire()
{
	//Here be some weapon specific code, but for now, let's just draw a line and see what we hit

	//One: Trace a ray between the player and the nearest wall
	//Two: Check if any one sprite could have been hit
	SideCoords s;
	double w_dist = DBL_MAX;

	if (IntersectWall(&s, { player.x, player.y }, player.angle, HitscanSolid))
		w_dist = SDL_sqrt(SDL_pow(s.p_x, 2) + SDL_pow(s.p_y, 2));

	SDL_assert(w_dist < DBL_MAX);
	SDL_Log("%d", s.side->type);
	
}

void UpdatePlayer(u32 delta)
{
	SetPlayerMovement(delta);

	//Trace a line along the 
	if (input_tf.use) Use();
	if (input_tf.fire) Fire();

	player.speed = accel_forward * delta;
	player.strafe = accel_strafe * delta;

	u32 moveFlags = (noclip ? 0 : 3);
	MoveActor(&player, delta, moveFlags);
	ActorNormalizeAngle(&player);
}

inline void SetPlayerMovement(u32 delta)
{
	if (input.turn_left)
		player.angle += TURNSPEED*delta;
	if (input.turn_right)
		player.angle -= TURNSPEED*delta;

	if (input.forward)
	{
		accel_forward += .04*delta;
		if (accel_forward > MAXSPEED) accel_forward = MAXSPEED;
	}
	else if (input.backward)
	{
		accel_forward -= .04*delta;
		if (accel_forward < -MAXSPEED) accel_forward = -MAXSPEED;
	}
	else if (accel_forward > 0)
	{
		accel_forward -= .002 * delta;
		if (accel_forward < 0) accel_forward = 0;
	}
	else
	{
		accel_forward += .002*delta;
		if (accel_forward > 0) accel_forward = 0;
	}

	if (input.strafe_left)
	{
		accel_strafe += .01*delta;
		if (accel_strafe > MAXSTRAFE) accel_strafe = MAXSTRAFE;
	}
	else if (input.strafe_right)
	{
		accel_strafe -= .01*delta;
		if (accel_strafe < -MAXSTRAFE) accel_strafe = -MAXSTRAFE;
	}
	else if (accel_strafe > 0)
	{
		accel_strafe -= .0018 * delta;
		if (accel_strafe < 0) accel_strafe = 0;
	}
	else
	{
		accel_strafe += .0018*delta;
		if (accel_strafe > 0) accel_strafe = 0;
	}
}
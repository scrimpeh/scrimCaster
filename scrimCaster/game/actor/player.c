#include <game/actor/player.h>

#include <game/gameobjects.h>
#include <input/input.h>
#include <map/map.h>
#include <map/mapupdate.h>
#include <util/mathutil.h>

bool noclip = false;

double accel_forward = 0;
double accel_strafe = 0;

#define TURNSPEED .2
#define MAXSPEED .38
#define MAXSTRAFE .35

#define USELENGTH (M_CELLSIZE / 2)

static bool player_use_has_intercept;
static g_intercept player_use_intercept;

static bool player_fire_has_intercept;
static g_intercept player_fire_intercept;

void player_make(ac_actor* ac, m_obj* obj)
{
	watch_add_new(3, WCH_F64, "x: ", &ac->x, WCH_F64, ", y: ", &ac->y, WCH_F64, ", angle: ", &ac->angle);
	ac->type = obj->type;
	ac->x = obj->x;
	ac->y = obj->y;
	ac->angle = obj->angle;
}

static void player_use()
{
	const ac_actor* player = ac_get_player();
	player_use_has_intercept = false;
	g_cast(player->x, player->y, TO_RADF(player->angle), player_use_check_intercept);
	if (player_use_has_intercept)
	{
		const float distance = math_dist_f(player->x, player->y, player_use_intercept.x, player_use_intercept.y);
		if (distance < USELENGTH)
		{
			m_side* side = m_get_side(player_use_intercept.map_x, player_use_intercept.map_y, player_use_intercept.orientation);
			if (side->target)
				mu_activate_tag(side->target);
		}
	}
}

static bool player_use_check_intercept(const g_intercept* intercept)
{
	// The ray will happily travel any distance. That's fine, we need to render it anyway.
	player_use_has_intercept = true;
	SDL_memcpy(&player_use_intercept, intercept, sizeof(g_intercept));
	return false;
}

static void player_fire()
{
	const ac_actor* player = ac_get_player();

	// Here be some weapon specific code, but for now, let's just draw a line and see what we hit
	// One: Trace a ray between the player and the nearest wall
	// Two: Check if any one sprite could have been hit
	player_use_has_intercept = false;
	g_cast(player->x, player->y, TO_RADF(player->angle), player_use_check_intercept);
	if (player_use_has_intercept)
	{
		// ...
	}
}

static bool player_shoot_check_intercept(const g_intercept* intercept)
{
	// The ray will happily travel any distance. That's fine, we need to render it anyway.
	player_fire_has_intercept = true;
	SDL_memcpy(&player_fire_intercept, intercept, sizeof(g_intercept));
	return intercept->type == G_INTERCEPT_NON_SOLID;
}


bool player_update(ac_actor* ac, u32 delta)
{
	player_set_movement(delta);

	if (input_tf.use) 
		player_use();
	if (input_tf.fire) 
		player_fire();

	ac->speed = accel_forward * delta;
	ac->strafe = accel_strafe * delta;

	const u32 flags = noclip ? 0 : (AC_MOVE_COLLIDE_WALL | AC_MOVE_COLLIDE_ACTOR);
	ac_move(ac, delta, flags);

	// The player may never disappear from the map
	return false;
}

static inline void player_set_movement(u32 delta)
{
	ac_actor* player = ac_get_player();
	if (input.turn_left)
		player->angle += TURNSPEED * delta;
	if (input.turn_right)
		player->angle -= TURNSPEED * delta;

	if (input.forward)
	{
		accel_forward += .04 * delta;
		if (accel_forward > MAXSPEED)
			accel_forward = MAXSPEED;
	}
	else if (input.backward)
	{
		accel_forward -= .04 * delta;
		if (accel_forward < -MAXSPEED)
			accel_forward = -MAXSPEED;
	}
	else if (accel_forward > 0)
	{
		accel_forward -= .002 * delta;
		if (accel_forward < 0)
			accel_forward = 0;
	}
	else
	{
		accel_forward += .002 * delta;
		if (accel_forward > 0)
			accel_forward = 0;
	}

	if (input.strafe_left)
	{
		accel_strafe += .01 * delta;
		if (accel_strafe > MAXSTRAFE)
			accel_strafe = MAXSTRAFE;
	}
	else if (input.strafe_right)
	{
		accel_strafe -= .01 * delta;
		if (accel_strafe < -MAXSTRAFE)
			accel_strafe = -MAXSTRAFE;
	}
	else if (accel_strafe > 0)
	{
		accel_strafe -= .0018 * delta;
		if (accel_strafe < 0)
			accel_strafe = 0;
	}
	else
	{
		accel_strafe += .0018 * delta;
		if (accel_strafe > 0)
			accel_strafe = 0;
	}
}
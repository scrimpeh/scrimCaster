#include <player.h>

#include <input.h>
#include <map.h>
#include <maputil.h>
#include <mapupdate.h>
#include <util/mathutil.h>

extern Input input, input_tf;
Actor player;

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


void player_spawn()
{
	watch_add_new(3, WCH_F64, "x: ", &player.x, WCH_F64, ", y: ", &player.y, WCH_F32, ", angle: ", &player.angle);
	player.type = PLAYER;
	player.x = 50;
	player.y = 50;
	player.angle = 0;
}

static void player_use()
{
	player_use_has_intercept = false;
	g_cast(player.x, player.y, TO_RADF(player.angle), player_use_check_intercept);
	if (player_use_has_intercept)
	{
		const float distance = math_dist_f(player.x, player.y, player_use_intercept.x, player_use_intercept.y);
		if (distance < USELENGTH)
		{
			Side* side = map_get_side(player_use_intercept.map_x, player_use_intercept.map_y, player_use_intercept.orientation);
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
	// Here be some weapon specific code, but for now, let's just draw a line and see what we hit

	// One: Trace a ray between the player and the nearest wall
	// Two: Check if any one sprite could have been hit
	player_use_has_intercept = false;
	g_cast(player.x, player.y, TO_RADF(player.angle), player_use_check_intercept);
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


void player_update(u32 delta)
{
	player_set_movement(delta);

	if (input_tf.use) 
		player_use();
	if (input_tf.fire) 
		player_fire();

	player.speed = accel_forward * delta;
	player.strafe = accel_strafe * delta;

	u32 moveFlags = noclip ? 0 : 3;
	MoveActor(&player, delta, moveFlags);
}

static inline void player_set_movement(u32 delta)
{
	if (input.turn_left)
		player.angle += TURNSPEED * delta;
	if (input.turn_right)
		player.angle -= TURNSPEED * delta;

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
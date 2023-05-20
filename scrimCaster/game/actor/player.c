#include <game/actor/player.h>

#include <game/gameobjects.h>
#include <input/input.h>
#include <map/map.h>
#include <map/mapupdate.h>
#include <util/mathutil.h>

// Debug
#include <map/block/block_iterator.h>
#include <render/decal.h>

bool player_noclip = false;

double player_accel_forward = 0;
double player_accel_strafe = 0;

#define PLAYER_TURNSPEED .2
#define PLAYER_MAXSPEED .38
#define PLAYER_MAXSTRAFE .35

#define PLAYER_MAXSPEED_WALK .18
#define PLAYER_MAXSTRAFE_WALK .15

#define PLAYER_ACCEL .04
#define PLAYER_ACCEL_STRAFE .01
#define PLAYER_ACCEL_WALK .02
#define PLAYER_ACCEL_STRAFE_WALK .01

#define PLAYER_FRICTION .002
#define PLAYER_FRICTION_STRAFE .0018

#define PLAYER_USELENGTH (M_CELLSIZE / 2)

static bool player_use_has_intercept;
static g_intercept player_use_intercept;

static bool player_fire_has_intercept;
static g_intercept player_fire_intercept;

static u32 _block_object_count = 0;

void player_make(ac_actor* ac, m_obj* obj)
{
	watch_add_new(1, WCH_U32, "objects: ", &_block_object_count);
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
		if (distance < PLAYER_USELENGTH)
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
	watch_add_new(3, WCH_F64, "x: ", &ac->x, WCH_F64, ", y: ", &ac->y, WCH_F64, ", angle: ", &ac->angle);
	_block_object_count = 0;
	block_iterator* iter = block_iterator_make_actor(BLOCK_TYPE_DECAL_SIDE, ac);
	r_decal_world* decal = block_iterator_next(iter);
	while (decal)
	{
		//if (r->reference != ac)
		_block_object_count++;
		decal = block_iterator_next(iter);
	}
	block_iterator_free(iter);

	player_set_movement(delta);

	if (input_tf.use) 
		player_use();
	if (input_tf.fire) 
		player_fire();

	ac->speed = player_accel_forward * delta;
	ac->strafe = player_accel_strafe * delta;

	const u32 flags = player_noclip ? 0 : (AC_MOVE_COLLIDE_WALL | AC_MOVE_COLLIDE_ACTOR);
	ac_move(ac, delta, flags);

	// The player may never disappear from the map
	return false;
}

static inline void player_set_movement(u32 delta)
{
	ac_actor* player = ac_get_player();

	const double turnspeed = PLAYER_TURNSPEED;
	if (input.turn_left)
		player->angle += turnspeed * delta;
	if (input.turn_right)
		player->angle -= turnspeed * delta;

	const double maxspeed = input.walk ? PLAYER_MAXSPEED_WALK : PLAYER_MAXSPEED;
	const double accel = input.walk ? PLAYER_ACCEL_WALK : PLAYER_ACCEL;
	const double friction = PLAYER_FRICTION;
	if (input.forward)
		player_accel_forward = SDL_min(player_accel_forward + accel * delta, maxspeed);
	else if (input.backward)
		player_accel_forward = SDL_max(player_accel_forward - accel * delta, -maxspeed);
	else if (player_accel_forward > 0)
		player_accel_forward = SDL_max(player_accel_forward - friction * delta, 0);
	else
		player_accel_forward = SDL_min(player_accel_forward + friction * delta, 0);

	const double accel_strafe = input.walk ? PLAYER_ACCEL_STRAFE_WALK : PLAYER_ACCEL_STRAFE;
	const double maxstrafe = input.walk ? PLAYER_MAXSTRAFE_WALK : PLAYER_MAXSTRAFE;
	const double friction_strafe = PLAYER_FRICTION_STRAFE;
	if (input.strafe_left)
		player_accel_strafe = SDL_min(player_accel_strafe + accel_strafe * delta, maxstrafe);
	else if (input.strafe_right)
		player_accel_strafe = SDL_max(player_accel_strafe - accel_strafe * delta, -maxstrafe);
	else if (player_accel_strafe > 0)
		player_accel_strafe = SDL_max(player_accel_strafe - friction_strafe * delta, 0);
	else
		player_accel_strafe = SDL_min(player_accel_strafe + friction_strafe * delta, 0);
}
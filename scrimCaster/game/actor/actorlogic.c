#include <game/actor/actorlogic.h>

#include <game/actor/actorcontainers.h>
#include <game/actor/player.h>

ac_list ac_actors;

static const ac_logic AC_LOGIC[] =
{
	[AC_DUMMY]       = { ac_logic_make_dummy, ac_logic_update_dummy },
	[AC_PLAYER]      = { player_make,         player_update         },
	[AC_DUMMY_ENEMY] = { ac_logic_make_dummy, ac_logic_update_dummy },
	[AC_PILLAR]      = { ac_logic_make_dummy, ac_logic_update_dummy }
};

void ac_make(ac_actor* ac, m_obj* obj)
{
	AC_LOGIC[obj->type].constructor(ac, obj);
}

bool ac_update(ac_actor* ac, u32 delta)
{
	return AC_LOGIC[ac->type].updater(ac, delta);
}

static void ac_logic_make_dummy(ac_actor* ac, m_obj* obj)
{
	SDL_memset(ac, 0, sizeof(ac_actor));
	ac->type = obj->type;
	ac->x = obj->x;
	ac->y = obj->y;
	ac->angle = obj->angle;
	ac->speed = 0;
	ac->strafe = 0;
}

static bool ac_logic_update_dummy(ac_actor* ac, u32 delta)
{
	return false;	
}

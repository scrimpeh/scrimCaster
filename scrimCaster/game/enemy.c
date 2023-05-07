#include <game/enemy.h>

#include <game/actor/actor.h>
#include <game/actor/actorcontainers.h>

// Some basic definitions or somesuch
ActorVector levelEnemies;

/* Enemy update functions will return a bool
 * indicating whether or not they have disappeared for the map.
 * In most cases, this should be FALSE 
 */
static bool UpdateZeroEnemy(u32 timeStep, ac_actor* a)
{
	return false;
}

static bool UpdateDummyEnemy(u32 timeStep, ac_actor* a)
{
	if (ac_move(a, timeStep, 3))
	{
		a->speed *= -1;
	}
	return false;
}

static bool (*update_enemy_functions[])(u32, ac_actor*) =
{
	UpdateZeroEnemy,
	UpdateDummyEnemy
};

void UpdateEnemies(u32 timeStep)
{
	u32 remove_count = 0;
	for (u32 i = 0; i < levelEnemies.count; ++i)
	{
		ac_actor* cur_enemy = levelEnemies.content[i];

		if (update_enemy_functions[cur_enemy->type](timeStep, cur_enemy))
		{
			SDL_free(cur_enemy);	
			--levelEnemies.count;
			++remove_count;
		}
		else levelEnemies.content[i - remove_count] = cur_enemy;
	}
}
#include "enemy.h"

#include "actor.h"
#include "actorcontainers.h"

#include "SDL/SDL_assert.h"

//Some basic definitions or somesuch
ActorVector levelEnemies;

/* Enemy update functions will return a bool
 * indicating whether or not they have disappeared for the map.
 * In most cases, this should be FALSE 
 */
static bool UpdateZeroEnemy(u32 timeStep, Actor* a)
{
	return false;
}

static bool UpdateDummyEnemy(u32 timeStep, Actor* a)
{
	if (MoveActor(a, timeStep, 3))
	{
		a->speed *= -1;
	}
	return false;
}

static bool (*update_enemy_functions[])(u32, Actor*) =
{
	UpdateZeroEnemy,
	UpdateDummyEnemy
};

void UpdateEnemies(u32 timeStep)
{
	u32 remove_count = 0;
	for (u32 i = 0; i < levelEnemies.count; ++i)
	{
		Actor* cur_enemy = levelEnemies.content[i];

		SDL_assert(cur_enemy->actor_index < SDL_arraysize(update_enemy_functions));

		if (update_enemy_functions[cur_enemy->actor_index](timeStep, cur_enemy))
		{
			SDL_free(cur_enemy);	
			--levelEnemies.count;
			++remove_count;
		}
		else levelEnemies.content[i - remove_count] = cur_enemy;
	}
}
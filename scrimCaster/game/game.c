#include <game/game.h>

#include <game/actor/player.h>
#include <game/camera.h>
#include <game/gameobjects.h>
#include <input/input.h>
#include <input/mouselook.h>
#include <map/map.h>
#include <map/mapupdate.h>

// On game Objects:
/* Do we want a unified Actor system, or break up several types
 * of actors into several types? Here's a chart to illustrate

          | Creation | Moving | Solid | Disappear/Die |
   Player | Game     | Yes    | Yes   | No*           |
   LvlObj | Level    | No     | Some  | No            |
   Pickup | Level    | Maybe  | No    | Disappear     |
   Enemy  | Level    | Yes    | Yes   | Die**         |
   Proj   | Temp     | Yes    | No    | Disappear     |
   Prtcls | Temp     | No     | No    | Disappaer     |
  
   * - Results in a failure state instead.
   ** - May leave behind death sprite.

 * Some actors may spawn other actors, like projectiles and particle effects on
 * certain conditions. The engine must accomodate this.
 *
 * Do we want static storage, dynamic storage?
 * When an enemy dies, is it still a (dead) enemy, or does its corpse turn
 * into a level object?
 *
 * Potential pitfalls to think about -before- trying to implement!:
 * 1.Are objects stored as one heterogenous type or as several different ones
 * - If heterogenous types are used, how will it be implemented?
 * - C structs, C++ classes w/ inheritance?
 * 2.Are all objects stored in one contiguous location in memory?
 * - Is it allocated at launch, level load, or dynamically expanded
 * - Array, custom container, STL container? std::list, std::vector?
 *
 * Potential areas all of this has implications on:
 * - Map Loading
 * - Game logic
 * - Actor collision
 * - Sprite drawing
 *
 * RAM is cheap. Arguably, wasting some extra space
 * to store information for objects that don't need it might be preferrable to 
 * having several cases for each type of object we want to deal with!
 */

const u32 TIMESTEP_MAX = 128;
u64 ticks;

bool game_menu;

void SetMenu(bool open)
{
	mouselook_suspend(open);
	game_menu = open;
}

i32 game_init()
{
	game_free();
	ac_load(m_map.objects, m_map.obj_count);
	SDL_assert(ac_actors.count > 0);
	SDL_assert(ac_actors.first->actor.type == AC_PLAYER);
	return 0;
}

void game_free()
{

}

void UpdateGame(u32 delta)
{
	// Make sure we don't skip too much
	if (delta > TIMESTEP_MAX) 
		delta = TIMESTEP_MAX;

	if (game_menu)
	{
		if (input_tf.pause)
			SetMenu(false);
	}
	else
	{
		ticks += delta;
		mu_update(delta);
		ac_update_objects(delta);
		UpdateCamera(delta);
		
		if (input_tf.pause)
			SetMenu(true);
	}
}
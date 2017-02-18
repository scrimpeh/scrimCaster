#include "types.h"
#include "game.h"
#include "map.h"
#include "mapupdate.h"
#include "player.h"
#include "camera.h"
#include "input.h"
#include "mouselook.h"
#include "actorcontainers.h"
#include "enemy.h"

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

ActorList tempEnemies;
ActorList particles;
ActorList projectiles;

//Gamestate variables
bool loadMap = true;
bool inGame = false;
bool inMenu = true;

extern Input input, input_tf;

void SetMenu(bool open)
{
	SuspendMouselook(open);
	inMenu = open;
}

void UpdateGame(u32 timeStep)
{
	//Make sure we don't skip too much
	if (timeStep > TIMESTEP_MAX) timeStep = TIMESTEP_MAX;

	if (loadMap)
	{
		LoadMap();
		PlayerStartMap();
		loadMap = false;
		SetMenu(false);
		inGame = true;
		return;
	}

	if (inMenu)
	{
		if (input_tf.pause)
			SetMenu(false);
	}
	else if (inGame)
	{
		ticks += timeStep;
		UpdateEnemies(timeStep);
		UpdateSides(timeStep);
		UpdatePlayer(timeStep);
		UpdateCamera(timeStep);
		
		if (input_tf.pause)
			SetMenu(true);
	}
}
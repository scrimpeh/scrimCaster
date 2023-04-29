#pragma once

#include "common.h"

// A cell is the basic unit a map is comprised of.
// Each cell has a uniform size in game units and four walls on the interior side
// that can either have a type or nothing. Similar to Doom, cells and tags can have types
// that control dynamic behaviour. Note that at runtime, no parameters are considered static,
// all should be modifiable.

// Any side can be universally identified by an offset on the map,
// which is floor(x / map.w) + (x mod map.w) 

// Note: All directionality in this game is East - North - West - South
// 0° degrees in a circle is pointing east as well.
typedef u8 m_orientation;

#define M_EAST  0
#define M_NORTH 1
#define M_WEST  2
#define M_SOUTH 3

// Define The size of one individual cell in game units.
#define M_CELLSIZE 64
// The height of one individual cell in game units.
#define M_CELLHEIGHT 64

//Possible flags. Note: Which ones are actually gonna be implemented, I'm not sure
//Just balling around ideas so far
typedef enum m_side_flags : u32
{
	PASSABLE =         0x0001,
	TRANSLUCENT =      0x0002,
	BULLETS_PASS =     0x0004,
	PROJECTILES_PASS = 0x0008,
	SWITCH_ONCE =      0x0010,
	SWITCH_MULTIPLE =  0x0020,
	DOOR_V =           0x0040,
	DOOR_H =           0x0080,
	DOOR_ANIM =        0x0100,
	SCROLL_V =		   0x0200,
	SCROLL_H =         0x0400,
	MIRR_H =           0x0800,
	MIRR_V =           0x1000
} m_side_flags;

// The floor flags. Some flags may be contradictory, e.g. a "Multiple" Trigger
// overrides a "Once" trigger. In this case, a certain flag wins.
typedef enum
{
	M_FLOOR_TRIGGER_ONCE     = 0x0001,
	M_FLOOR_TRIGGER_MULTIPLE = 0x0002
} m_floor_flags;

typedef struct
{
	u16 floor_type;
	u16 ceil_type;
	u32 target;
	m_floor_flags flags;
} m_flat;

typedef enum DoorFlags
{
	PLAYER_ACTIVATE = 1,
	MONSTER_ACTIVATE = 2
} DoorFlags;

typedef struct DoorParams
{
	u8 openspeed, closespeed; //the ticks needed to reach one increment in game units
	u8 staytime;			  //the amount of (half-)seconds that a door stays open
	DoorFlags door_flags;

	u8 state; //0: closed/inactive, 1: opening, 2: open, 3: opened
	i16 scroll; //from 0 - 255, represents how open the door is, with 128 representing half
	i16 timer_staycounter;
	u8 timer_ticks;
} DoorParams;

typedef struct Side
{
	u16 type;
	u32 tag;
	u32 target;
	u8 state;
	bool solid;
	m_side_flags flags;
	union
	{
		struct			//default params
		{
			u8 scroll_x, scroll_y;
			u32 param1, param2;
		};

		DoorParams door;
	};
} Side;

typedef struct Cell
{
	m_flat flat;
	Side e;
	Side n;
	Side w;
	Side s;
} Cell;

Side* m_cell_get_side(Cell* cell, m_orientation orientation);

typedef Side** m_tag_array;
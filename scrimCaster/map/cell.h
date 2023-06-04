#pragma once

#include <common.h>

// A cell is the basic unit a map is comprised of.
// Each cell has a uniform size in game units and four walls on the interior side
// that can either have a type or nothing. Similar to Doom, cells and tags can have types
// that control dynamic behaviour. Note that at runtime, no parameters are considered static,
// all should be modifiable.

// Any side can be universally identified by an offset on the map,
// which is floor(i / map.w) + (i mod map.w) 

// Note: All directionality in this game is East - North - West - South
// 0° degrees in a circle is pointing east as well.
typedef enum
{
	M_EAST = 0,
	M_NORTH = 1,
	M_WEST = 2,
	M_SOUTH = 3,

	M_FLOOR = 4,
	M_CEIL = 5
} m_orientation;

// The size of one individual cell in game units.
#define M_CELLSIZE 64
// The height of one individual cell in game units.
#define M_CELLHEIGHT 64

// Possible flags. Note: Which ones are actually gonna be implemented, I'm not sure
// Just balling around ideas so far
typedef enum
{
	PASSABLE =           0x0001,
	TRANSLUCENT =        0x0002,
	BULLETS_PASS =       0x0004,
	PROJECTILES_PASS =   0x0008,
	SWITCH_ONCE =        0x0010,
	SWITCH_MULTIPLE =    0x0020,
	DOOR_V =             0x0040,
	DOOR_H =             0x0080,
	DOOR_ANIM =          0x0100,
	SCROLL_V =		     0x0200,
	SCROLL_H =           0x0400,
	MIRR_H =             0x0800,
	MIRR_V =             0x1000,
	BLOCK_SMOOTH_LIGHT = 0x2000,
} m_side_flags;

typedef enum
{
	M_FLAT_FLIP_V =     0x0001,
	M_FLAT_FLIP_H =     0x0002,
	M_FLAT_ROTATE_90 =  0x0004,
	M_FLAT_ROTATE_180 = 0x0008
} m_flat_flags;

typedef struct
{
	u16 type;
	m_flat_flags flags;
} m_flat;

// Parameters for a door
typedef struct
{
	u8 openspeed;
	u8 closespeed;
	u8 staytime;

	u8 state;
	// How open a door is, with 0 being completely closed, and 64 being fully opened
	i8 scroll;
	i16 timer_staycounter;
	u8 timer_ticks;
} m_side_door_params;

typedef struct
{
	u16 type;
	u32 tag;
	u32 target;
	u8 state;
	bool solid;
	m_side_flags flags;
	m_side_door_params door;
} m_side;

typedef struct
{
	// Flats
	m_flat floor;
	m_flat ceil;

	// Sides
	m_side e;
	m_side n;
	m_side w;
	m_side s;

	// Cell info
	u8 brightness;
	u32 target;
} m_cell;

typedef struct
{
	i16 x;
	i16 y;
	m_orientation orientation;
} m_side_id;

m_side* m_cell_get_side(m_cell* cell, m_orientation orientation);

m_orientation m_get_opposite_orientation(m_orientation orientation);
i16 m_get_next_x(i16 x, m_orientation orientation);
i16 m_get_next_y(i16 y, m_orientation orientation);

typedef m_side** m_tag_array;

#pragma once

// This is the new unified geometry handling module
// Everything that needs to interact with the map, i.e., rendering, interacting with walls or shooting
// should go through this module

// A general note on coordinates:
// Starting now, I want to have a unified naming system for coordinates

// World coordinates should be wx (horizontal axis, west -> east), wy (vertical axis, north -> south), and wz (height, ceiling -> floor)
// Map coordinates should be mx and my (dito)
// Screen coordinates should be sx, sy, and sz (depth)

#include <common.h>

#include <map/cell.h>

typedef u8 g_orientation;

#define ORIENTATION_NORTH 0x1
#define ORIENTATION_EAST 0x2
#define ORIENTATION_SOUTH 0x0
#define ORIENTATION_WEST 0x0


typedef enum g_intercept_type
{
	// The ray ended in the void. This is generally not supposed to happen, unless the ray started outside from the map to begin with, which it never should
	G_INTERCEPT_VOID = 0,

	// The ray ended in a solid
	G_INTERCEPT_SOLID = 1,

	// The ray intersected with a transparent wall and kept going
	G_INTERCEPT_NON_SOLID = 2,
} g_intercept_type;


// A basic struct describing an intersection with a ray and a wall
typedef struct g_intercept {
	m_orientation orientation;
	g_intercept_type type;
	angle_rad_f angle;
	float wx;
	float wy;
	i16 mx;
	i16 my;
	u8 column;
} g_intercept;

// Callback that handles encountered intercepts, front-to-back.. The usual rendering pipeline involves
// adding them to a stack and rendering them back-to-front
typedef bool (*g_intercept_collector)(const g_intercept*);

// The main catch-all ray casting function
void g_cast(float origin_x, float origin_y, angle_rad_f angle, g_intercept_collector intercept_collector);

static inline g_orientation g_get_orientation(angle_rad_f angle);
static inline bool g_is_north(g_orientation orientation);
static inline bool g_is_east(g_orientation orientation);
static inline bool g_is_west(g_orientation orientation);
static inline bool g_is_south(g_orientation orientation);

static inline g_intercept_type g_get_intercept_type(const m_side* side, bool is_edge);

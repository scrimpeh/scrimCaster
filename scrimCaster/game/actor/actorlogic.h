#pragma once

#include <common.h>

// Defines actor logic
// An actor a number of callbacks for a fixed number of events

// Currently, these are:
// - Init
// - Move

// Other potential types would be "Hurt" and "Dead"

#include <game/actor/actor.h>
#include <game/actor/actorcontainers.h>
#include <map/mapobject.h>

// Actor behaviour callbacks

// Constructs an actor from a map object
typedef void (*ac_constructor)(ac_actor* ac, const m_obj* obj);

// Returns true, if the actor should be removed, false otherwise
typedef bool (*ac_updater)(ac_actor* ac, u32 delta);

typedef struct
{
	ac_constructor constructor;
	ac_updater updater;
} ac_logic;

void ac_make(ac_actor* ac, m_obj* obj);
bool ac_update(ac_actor* ac, u32 delta);

static void ac_logic_make_dummy(ac_actor* ac, m_obj* obj);
static bool ac_logic_update_dummy(ac_actor* ac, u32 delta);

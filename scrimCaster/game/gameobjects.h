#pragma once

#include <common.h>

// This module is responsible for holding the currently loaded actors

#include <game/actor/actor.h>
#include <game/actor/actorcontainers.h>
#include <map/map.h>
#include <map/mapobject.h>

extern ac_list ac_actors;

i32 ac_load(const m_obj* obj, u32 count);
void ac_destroy();

void ac_create(const m_obj* obj);
void ac_update_objects(u32 delta);

ac_actor* ac_get_player();
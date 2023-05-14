#pragma once

#include <common.h>

#include <game/actor/actor.h>

extern float viewport_angle;
extern float viewport_x;
extern float viewport_y;

void cam_set_actor(ac_actor* actor);
ac_actor* cam_get_actor();
void cam_update(u32 delta);
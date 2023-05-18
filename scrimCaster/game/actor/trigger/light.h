#pragma once

#include <common.h>

#include <game/actor/actor.h>

bool ac_logic_light_flicker(ac_actor* ac, u32 delta);

static void ac_logic_light_flicker_set_brightness_cell(i16 mx, i16 my, u8 brightness);
static void ac_logic_light_flicker_set_brightness(const ac_actor* ac, u8 brightness);
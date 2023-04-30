#pragma once

#include <common.h>

#include <SDL/SDL_video.h>

SDL_Surface* gfx_load(const char* filename);

i32 gfx_load_global();
void gfx_unload();

static i32 gfx_load_sprites();
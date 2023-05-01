#pragma once

// A skybox that is a flat texture that is drawn as a backdrop for the level.
// There are a number of fixed skyboxes which are loaded at game load time, which are indexed by their numeric index.
// By default, a map has one skybox associated with it, but optionally, one could use triggers to change the skybox on the fly.

// For sides, there is a special sykbox texture (1). If the renderer encounters this texture, it will draw a slice of the skybox instead of the regular wall textures.
// For floors, if the color to be drawn equals the color key (#FF00FF), the appropriate skybox pixel will be drawn instead.

#include <common.h>

#include <render/texture.h>

#include <SDL/SDL_video.h>

#define TX_SKY 1

typedef struct
{
	u16 w;
	u16 h;
	tx_block data;
} r_skybox;

i32 r_sky_load_global();
void r_sky_unload();

static i32 r_sky_load(r_skybox* sky, const char* path);

void r_sky_set_current(i32 sky);
void r_sky_draw(SDL_Surface* target, u16 col, u16 y_start, u16 y_end, angle_rad_f angle);
u32 r_sky_get_pixel(u16 y, angle_rad_f angle);
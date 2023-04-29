#pragma once

#include <common.h>

#include <SDL/SDL_video.h>

SDL_Surface* gfx_load(const char* filename);
i32 LoadGlobalSurfaces();
i32 LoadMapTexture(const char* filename);
void UnloadMapTextures();
void UnloadAllTextures();
static i32 LoadWorldSprites();
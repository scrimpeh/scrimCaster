#pragma once

#include <common.h>

#include <SDL/SDL_render.h>


static SDL_Surface* LoadSurface(const char* filename);
i32 LoadGlobalSurfaces();
i32 LoadMapTexture(const char* filename);
void UnloadMapTextures();
void UnloadAllTextures();
static i32 LoadWorldSprites();
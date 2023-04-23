#pragma once

#include "common.h"

#include "geometry.h"

#include "SDL/SDL_render.h"

inline void DrawPixel(const SDL_Surface* const surface, const SDL_Point p, const u32 col);
inline void DrawPixelBounds(const SDL_Surface* const surf, const SDL_Point p, const u32 col);
void DrawLine(const SDL_Surface* const surf, const SDL_Point p1, const SDL_Point p2, const u32 col);
void DrawLineBounds(const SDL_Surface* const surf, const SDL_Point p1, const SDL_Point p2, const u32 col);

i32 r_init(u16 w, u16 h);
void r_close();
void RenderFrame();
void TextOut(SDL_Surface* surf, const char* str, i32 x, i32 y);

static void r_map_coordinate(float x, float y, SDL_Point* target);
static bool r_map_view_intersect(const g_intercept* intercept);
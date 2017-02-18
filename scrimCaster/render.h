#pragma once

#include "SDL/SDL_render.h"

inline void DrawPixel(const SDL_Surface* const surface, const SDL_Point p, const u32 col);
inline void DrawPixelBounds(const SDL_Surface* const surf, const SDL_Point p, const u32 col);
void DrawLine(const SDL_Surface* const surf, const SDL_Point p1, const SDL_Point p2, const u32 col);
void DrawLineBounds(const SDL_Surface* const surf, const SDL_Point p1, const SDL_Point p2, const u32 col);

i32 InitializeRenderer(u16 w, u16 h, u8 col = 1);
void CloseRenderer();
void RenderFrame();
void TextOut(SDL_Surface* surf, const char* str, i32 x, i32 y);

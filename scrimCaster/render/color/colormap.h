#pragma once

// This module deals with remapping colors for lighting / distance fog, etc

// For the purpose of the renderer, a color map is a list of set points imparting
// various hues at various distances. The color of a pixel at a given color at the color map
// is linearly interpolated at a distance

// All colors in this executable are 32-bit RGBA colors, which are 8 bits each.
// Alpha is handled separately as a float ranging from 0.0 to 1.0

#include <common.h>

#include <SDL/SDL_video.h>

typedef u32 cm_color;
typedef u8 cm_channel;

typedef struct
{
	cm_color color;
	float alpha;
} cm_alpha_color;

#define CM_R_MASK 0x00FF0000
#define CM_G_MASK 0x0000FF00
#define CM_B_MASK 0x000000FF

#define CM_R(color) (((color) & CM_R_MASK) >> 16)
#define CM_G(color) (((color) & CM_G_MASK) >> 8)
#define CM_B(color) (((color) & CM_B_MASK))

#define CM_GET(r, g, b) (((r) << 16) | ((g) << 8) | (b))

cm_color cm_map(cm_color source, cm_color target, float alpha);
cm_color cm_from_sdl_color(const SDL_Color* color);
void cm_to_sdl_color(SDL_Color* color, cm_color source);

static cm_channel cm_mix_channel(cm_channel source, cm_channel target, float alpha);

static bool r_clip_line(i32 w, i32 h, i32* x_a, i32* y_a, i32* x_b, i32* y_b);
static inline u8 r_clip_code(i32 w, i32 h, i32 x, i32 y);


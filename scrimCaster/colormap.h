#pragma once

// This module deals with remapping colors for lighting / distance fog, etc

// For the purpose of the renderer, a color map is a list of set points imparting
// various hues at various distances. The color of a pixel at a given color at the color map
// is linearly interpolated at a distance

// All colors in this executable are 32-bit RGBA colors, which are 8 bits each.
// Alpha is handled separately as a float ranging from 0.0 to 1.0

#include "common.h"

typedef u32 cm_color;
typedef u8 cm_channel;

#define CM_R_MASK 0x00FF0000
#define CM_G_MASK 0x0000FF00
#define CM_B_MASK 0x000000FF

#define CM_R(color) (((color) & CM_R_MASK) >> 16)
#define CM_G(color) (((color) & CM_G_MASK) >> 8)
#define CM_B(color) (((color) & CM_B_MASK))

#define CM_GET(r, g, b) (((r) << 16) | ((g) << 8) | (b))

cm_color cm_map(cm_color source, cm_color target, float alpha);

static cm_channel cm_mix_channel(cm_channel source, cm_channel target, float alpha);


#pragma once

#include <common.h>

#include <map.h>


// Textures are 64 x 64 PX bitmaps that are loaded from disk and drawn on the walls and floors of maps
// On disk, textures are stored in simple PNG files, which are then converted to an internal format for rendering
// As textures are drawn in vertical slices, the format also stores textures as vertical slices

// In a level, textures are referenced by their numeric index, which is simply the position of the texture in the texture set
// Multiple textures sets can be joined to form one large texture set, the texture indices are then adjusted accordingly

#define TX_SIZE 64

i32 tx_load(const Map* map);


void tx_unload();

// Gets a pointer to the pixel data for the texture at index, at the given position
// Automatically handles getting the correct vertical slice, but vertical mirroring needs to be done at the drawing side
// The purpose of this function is to shunt off all special jank cases into a separate function
// though somehow this needs to handle both slices and floors
const u32* tx_get_slice(const Side* side, u8 column);
const u32 tx_get_point(const m_flat* flat, u8 x, u8 y, bool floor);
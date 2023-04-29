#include <texture.h>

#include <SDL_video.h>

extern SDL_Surface* mapTextureBuffer[];

i32 tx_load(const Map* map)
{
	// TODO: texture loading
	return 0;
}

void tx_unload()
{

}

const u32* tx_get_slice(const Side* side, u8 column)
{
	// What was I thinking...
	const u16 type = side->type;
	const u8 tx_index = type & 0x00FF;
	const u8 tx_sheet = (type & 0xFF00) >> 8;
	const SDL_Surface* tx_surface = mapTextureBuffer[tx_sheet];

	const u16 textures_per_row = tx_surface->w / TX_SIZE;

	const u16 tile_y = tx_index / textures_per_row;
	const u16 tile_x = tx_index % textures_per_row;

	const u16 px_y = tile_y * TX_SIZE;
	const u16 px_x = tile_x * TX_SIZE + column;

	return (u32*)tx_surface->pixels + px_y * tx_surface->w + px_x;
}

const u32 tx_get_point(const m_flat* flat, u8 x, u8 y, bool floor)
{
	const u16 type = floor ? flat->floor_type : flat->ceil_type;
	const u8 tx_index = type & 0x00FF;
	const u8 tx_sheet = (type & 0xFF00) >> 8;
	const SDL_Surface* tx_surface = mapTextureBuffer[tx_sheet];

	const u16 textures_per_row = tx_surface->w / TX_SIZE;

	const u16 tile_y = tx_index / textures_per_row;
	const u16 tile_x = tx_index % textures_per_row;

	const u16 px_y = tile_y * TX_SIZE + y;
	const u16 px_x = tile_x * TX_SIZE + x;

	return *((u32*)tx_surface->pixels + px_y * tx_surface->w + px_x);
}
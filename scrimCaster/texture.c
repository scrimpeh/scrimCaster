#include <texture.h>

#include <gfxloader.h>

#include <SDL_video.h>

extern SDL_Surface* mapTextureBuffer[];

static const char* TX_PATH = "tx/map/";

#define TX_PATH_BUF_SIZE 512
#define TX_PX_SIZE (TX_SIZE * TX_SIZE)
char tx_path_buf[TX_PATH_BUF_SIZE];

tx_block tx_map_textures = NULL;

i32 tx_map_load(u32 count, const char** tx_set_names)
{
	tx_unload();

	// Load map textures

	// First of all, buffer all the raw images into a file
	SDL_Surface** surfaces = SDL_malloc(sizeof(SDL_Surface*) * count);
	if (!surfaces)
		return -1;

	u64 tx_count = 0;
	for (u32 i = 0; i < count; i++)
		surfaces[i] = NULL;
	for (u32 i = 0; i < count; i++)
	{
		tx_path_buf[0] = '\0';
		SDL_snprintf(tx_path_buf, TX_PATH_BUF_SIZE, "%s%s", TX_PATH, tx_set_names[i]);
		surfaces[i] = gfx_load(tx_path_buf);
		if (surfaces[i])
		{
			const u64 tx_columns = surfaces[i]->w / TX_SIZE;
			const u64 tx_rows = surfaces[i]->h / TX_SIZE;
			tx_count += tx_rows * tx_columns;
		}
		else
		{
			for (u32 j = 0; j < i; j++)
				SDL_FreeSurface(surfaces[j]);
			SDL_free(surfaces);
			return -2;
		}
	}

	// Now construct the strips
	tx_map_textures = SDL_malloc(sizeof(u32) * tx_count * TX_PX_SIZE);
	if (!tx_map_textures)
		return -3;

	u32 current_surface = 0;
	tx_block current_texture = tx_map_textures;
	for (u32 texture = 0; texture < tx_count; texture++)
	{
		const SDL_Surface* surface = surfaces[current_surface];
		SDL_Rect r;
		r.x = (texture % (surface->w / TX_SIZE)) * TX_SIZE;
		r.y = (texture / (surface->h / TX_SIZE)) * TX_SIZE;
		r.w = TX_SIZE;
		r.h = TX_SIZE;
		tx_copy(surface, &r, current_texture);
		current_texture += TX_PX_SIZE;
		if ((r.y + TX_SIZE) > surface->h)
			current_surface++;
	}

	for (u32 i = 0; i < count; i++)
		SDL_FreeSurface(surfaces[i]);
	SDL_free(surfaces);
	return 0;
}

static void tx_copy(const SDL_Surface* source, const SDL_Rect* r, tx_block target)
{
	u64 offs = 0;
	for (u32 x = r->x; x < r->x + r->w; x++)
		for (u32 y = r->y; y < r->y + r->h; y++)
			target[offs++] = *((u32*) source->pixels + y * source->w + x);
}

void tx_unload()
{
	SDL_free(tx_map_textures);
	tx_map_textures = NULL;
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
	const tx_block block = tx_map_textures + type * TX_PX_SIZE;
	const tx_slice slice = block + x * TX_SIZE;
	return *(slice + y);

	// const u8 tx_index = type & 0x00FF;
	// const u8 tx_sheet = (type & 0xFF00) >> 8;
	// const SDL_Surface* tx_surface = mapTextureBuffer[tx_sheet];
	// 
	// const u16 textures_per_row = tx_surface->w / TX_SIZE;
	// 
	// const u16 tile_y = tx_index / textures_per_row;
	// const u16 tile_x = tx_index % textures_per_row;

	// const u16 px_y = tile_y * TX_SIZE + y;
	// const u16 px_x = tile_x * TX_SIZE + x;
	// 
	// return *((u32*)tx_surface->pixels + px_y * tx_surface->w + px_x);
}

const void tx_strip_blit(tx_slice strip, u16 tx_start, u16 tx_end, SDL_Surface* target, i16 target_start, i16 target_end)
{
	// TODO
}


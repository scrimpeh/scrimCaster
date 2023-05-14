#include <map/map.h>

#include <game/actor/actor.h>
#include <map/mapobject.h>
#include <map/mapupdate.h>
#include <render/gfxloader.h>
#include <render/texture.h>
#include <render/skybox.h>

/* On scrolling doors:
 * When activated, a door switches from 'SOLID' into 'TRANSLUCENT' as well as other flags that need changing.
 * param1 shall define the scrollspeed (in terms of ticks per val)
 * while param2 shall define how long the door remains open (if at all), with all 0 meaning "forever"
 * we probably also need some status flags on the door to be shoehorned in
 *
 * scroll shall define "how open" the door is, i.e. it determines the ratio between open and not open
 */

m_map_data m_map;

const char* tx_sets[] = { "test.png" };


static m_cell cells[16][16];

static const m_obj objects[] =
{
	{ .type = AC_PLAYER, .x = 50,  .y = 50,  .angle = 0 },
	{ .type = AC_PILLAR, .x = 300, .y = 100, .angle = 0 },
	{ .type = AC_PILLAR, .x = 96,  .y = 480, .angle = 0 },
	{ .type = AC_PILLAR, .x = 448, .y = 320, .angle = 0 },
	{ .type = AC_PILLAR, .x = 448, .y = 256, .angle = 0 },
	{ .type = AC_PILLAR, .x = 448, .y = 192, .angle = 0 }
};

static const r_decal_world decals[] =
{
	{ .type = 1, .id = { .orientation = M_NORTH, .x = 2, .y = 0 }, .x = 32, .y = 16, .ttl = 0 },
	{ .type = 1, .id = { .orientation = M_FLOOR, .x = 4, .y = 2 }, .x = 48, .y = 16, .ttl = 0 },
	{ .type = 1, .id = { .orientation = M_CEIL,  .x = 4, .y = 2 }, .x = 32, .y = 32, .ttl = 0 }
};

u32 m_max_tag;
m_taglist* m_tags;
bool* m_tag_active;

m_cell* m_get_cell(u16 x, u16 y)
{
	SDL_assert(x < m_map.w && y < m_map.h);
	return &m_map.cells[m_map.w * y + x];
}

m_side* m_get_side(u16 x, u16 y, m_orientation o)
{
	return m_cell_get_side(m_get_cell(x, y), o);
}

m_side* m_get_side_from_id(m_side_id id)
{
	return m_cell_get_side(m_get_cell(id.x, id.y), id.orientation);
}

m_side* m_get_opposite_side(u16 x, u16 y, m_orientation o)
{
	switch (o)
	{
	case M_EAST:  return x == m_map.w - 1 ? NULL : m_get_side(x + 1, y, M_WEST);
	case M_NORTH: return y == 0 ?           NULL : m_get_side(x, y - 1, M_SOUTH);
	case M_WEST:  return x == 0 ?           NULL : m_get_side(x - 1, y, M_EAST);
	case M_SOUTH: return y == m_map.h - 1 ? NULL : m_get_side(x, y + 1, M_NORTH);
	default:
		SDL_assert(!"Invalid side!");
		return NULL;
	}
}

m_cell* m_get_next_cell(u16 x, u16 y, m_orientation o)
{
	return m_get_cell(m_get_next_x(x, o), m_get_next_y(y, o));
}

void m_load()
{
	u8 i, j, n;
	u32 target = 1;
	m_map.w = 16;
	m_map.h = 16;
	m_map.cells = cells[0];
	m_map.objects = &objects[0];
	m_map.obj_count = 6;
	m_map.info.texture_set_count = 1;
	m_map.info.texture_sets = tx_sets;
	m_map.info.sky = 0;
	m_map.decal_count = 3;
	m_map.decals = &decals[0];

	tx_map_load(m_map.info.texture_set_count, m_map.info.texture_sets);
	r_sky_set_current(m_map.info.sky);
	_m_flood_fill(0, 0, _m_flood_fill_cb_brightness, 255);

	for (i = 0; i < 8; ++i)
		for (j = 0; j < 8; ++j)
			for (n = 0; n < 4; ++n)
				m_cell_get_side(&cells[i][j], n)->type = 0;

	// Make rudimentary walls
	{
		for (u8 i = 0; i < 7; ++i)
		{
			cells[0][i].n.type = 2;
			cells[6][i].s.type = 2;
			cells[i][0].w.type = 2;
		}
		cells[4][0].w.type = 0;

		cells[0][0].w.type = 3;

		cells[0][6].e.type = 4;
		cells[2][6].e.type = 2;
		cells[6][6].e.type = 4;
		cells[5][6].e.type = 2;
		cells[2][7].w.type = 2;
		cells[5][7].w.type = 2;
		for (u8 i = 0; i < 4; ++i)
			cells[2 + i][9].e.type = 2;
		for (u8 i = 0; i < 3; ++i)
		{
			cells[2][7 + i].n.type = 2;
			cells[5][7 + i].s.type = 2;
		}
		cells[3][9].e.type = 6;
		cells[4][9].e.type = 6;

		cells[1][1].e.type = 4;
		cells[1][1].s.type = 2;
		cells[1][1].s.flags = SCROLL_H;
		// cellgrid[1][1].s.param1 = 8;
		cells[0][2].s.type = 6;
		cells[1][3].s.type = 2;
		cells[1][3].w.type = 2;
		cells[2][4].w.type = 6;
		cells[2][4].w.flags = (m_side_flags) (MIRR_H | SCROLL_H);
		// cellgrid[2][4].w.param1 = -8;
		cells[2][0].e.type = 6;
		cells[3][1].e.type = 2;
		cells[3][1].n.type = 2;
		cells[3][3].w.type = 2;
		cells[3][3].n.type = 2;
		cells[4][2].n.type = 6;
		cells[0][6].s.type = 2;
		cells[1][5].e.type = 4;
		cells[2][6].n.type = 2;

		cells[6][1].s.type = 0;
		cells[6][4].s.type = 0;
		cells[7][1].e.type = 4;
		cells[7][1].w.type = 2;
		cells[8][1].w.type = 2;
		cells[8][1].s.type = 2;
		cells[8][2].n.type = 2;
		cells[8][2].s.type = 2;
		cells[8][3].n.type = 2;
		cells[8][3].s.type = 4;
		cells[8][4].s.type = 4;
		cells[8][4].e.type = 2;
		cells[7][4].e.type = 2;
		cells[7][4].w.type = 4;

		cells[4][2].w.type = 3;
		cells[3][3].w.target = 1;
		cells[3][3].w.door.door_flags = PLAYER_ACTIVATE;

		{
			m_side side = { 0 };
			side.type = 3;
			side.flags = TRANSLUCENT;
			for (u8 y = 4; y <= 6; y++)
				_m_set_double_sided(1, y, M_EAST, &side);
		}

		cells[6][1].s.type = 6;
		cells[6][1].s.flags = DOOR_V;
		cells[6][1].s.door.state = 0;
		cells[6][1].s.door.openspeed = 12;
		cells[6][1].s.door.staytime = 12;
		cells[6][1].s.door.closespeed = 12;
		cells[6][1].s.door.door_flags = PLAYER_ACTIVATE;
		cells[6][1].s.tag = target++;
		cells[6][1].s.target = cells[6][1].s.tag;

		cells[7][1].n.type = 6;
		cells[7][1].n.flags = DOOR_V;
		cells[7][1].n.door.state = 0;
		cells[7][1].n.door.openspeed = 12;
		cells[7][1].n.door.staytime = 12;
		cells[7][1].n.door.closespeed = 12;
		cells[7][1].n.door.door_flags = PLAYER_ACTIVATE;
		cells[7][1].n.tag = cells[6][1].s.tag;
		cells[7][1].n.target = cells[6][1].s.tag;


		for (u8 x = 7; x < 12; x++)
			cells[0][x].n.type = TX_SKY;
		for (u8 x = 7; x < 12; x++)
			cells[8][x].s.type = TX_SKY;
		for (u8 y = 0; y < 9; y++)
			cells[y][11].e.type = TX_SKY;
		for (u8 y = 0; y < 9; y++)
			cells[y][7].w.type = 2;

		{
			m_side side = { 0 };
			side.type = 5;
			side.flags = PASSABLE | TRANSLUCENT;
			for (u8 y = 3; y <= 4; ++y)
				_m_set_double_sided(6, y, M_EAST, &side);
		}

		for (u8 i = 0; i < 4; ++i)
		{
			cells[2 + i][9].e.type = 3;
			cells[2 + i][9].e.flags = TRANSLUCENT;
		}
		for (u8 i = 0; i < 3; ++i)
		{
			cells[2][i + 7].n.type = 3;
			cells[2][i + 7].n.flags = TRANSLUCENT;
			cells[5][i + 7].s.type = 3;
			cells[5][i + 7].s.flags = TRANSLUCENT;
		}


		_m_flood_fill(0, 0, _m_flood_fill_cb_ceil, 7);
		_m_flood_fill(0, 0, _m_flood_fill_cb_floor, 8);
		_m_flood_fill(0, 0, _m_flood_fill_cb_brightness, 100);

		{
			m_side side = { 0 };
			side.type = 5;
			side.flags = PASSABLE | TRANSLUCENT;
			_m_set_double_sided(4, 6, M_SOUTH, &side);

		}

		cells[0][6].floor.type = 3;
		cells[0][6].ceil.type = 3;
		cells[0][6].w.flags = BLOCK_SMOOTH_LIGHT;
		cells[0][6].brightness = 128;

		_m_flood_fill(8, 2, _m_flood_fill_cb_ceil, 0);
		_m_flood_fill(8, 2, _m_flood_fill_cb_floor, 7);

		_m_flood_fill(4, 7, _m_flood_fill_cb_ceil, 0);
		_m_flood_fill(4, 7, _m_flood_fill_cb_brightness, 255);
		cells[5][6].brightness = 128;
		cells[6][6].brightness = 64;
		cells[6][5].brightness = 128;

		cells[4 - 1][4 + 1].brightness = 0;
		cells[3 - 1][4 + 1].brightness = 0;
		cells[4 - 1][3 + 1].brightness = 0;
		cells[3 - 1][3 + 1].brightness = 0;
	}



	cells[0][0].brightness = 64;
	cells[1][1].brightness = 64;
	cells[1][3].brightness = 64;
	cells[3][3].brightness = 64;
	cells[3][1].brightness = 64;


	cells[5][5].brightness = 0;

	cells[8][2].brightness = 128;
	cells[8][3].brightness = 128;
	m_create_tags();
};

void m_unload()
{
	mu_clear();
	m_destroy_tags();
	tx_unload();
};

static i32 m_create_tags()
{
	// Count the total number of tags and allocate an array to hold the tag information
	m_max_tag = 0;
	for (u16 y = 0; y < m_map.h; y++)
		for (u16 x = 0; x < m_map.w; x++)
			for (u8 orientation = 0; orientation < 4; orientation++)
				m_max_tag = SDL_max(m_get_side(x, y, orientation)->tag, m_max_tag);

	const u32 tag_count = m_max_tag + 1;

	m_tags = SDL_malloc(sizeof(m_taglist) * tag_count);
	if (!m_tags)
		return -1;

	// Find out how many sides are tagged per tag
	for (u32 i = 0; i <= m_max_tag; i++)
		m_tags[i].count = 0;
	for (u16 y = 0; y < m_map.h; y++)
		for (u16 x = 0; x < m_map.w; x++)
			for (u8 orientation = 0; orientation < 4; orientation++)
				m_tags[m_get_side(x, y, orientation)->tag].count++;

	// Allocate space for the tag lists
	for (u32 i = 1; i <= m_max_tag; i++)
		if (!(m_tags[i].sides = SDL_malloc(sizeof(m_side_id*) * m_tags[i].count)))
			return -2;

	// Now iterate through the map and build the side list for each tag
	u32* side_indices = SDL_malloc(sizeof(u32) * tag_count);
	for (u32 i = 1; i <= m_max_tag; i++)
		side_indices[i] = 0;

	for (u16 y = 0; y < m_map.h; y++)
	{
		for (u16 x = 0; x < m_map.w; x++)
		{
			for (u8 orientation = 0; orientation < 4; orientation++)
			{
				m_side* side = m_get_side(x, y, orientation);
				if (side->tag)
				{
					u32 next_tag = side_indices[side->tag]++;
					m_side_id id;
					id.x = x,
					id.y = y;
					id.orientation = orientation;
					m_tags[side->tag].sides[next_tag] = id;
				}
			}
		}
	}

	SDL_free(side_indices);

	// TEMP/TODO: This should be handled by mapupdate
	m_tag_active = SDL_malloc(sizeof(bool) * tag_count);
	if (!m_tag_active)
		return -3;
	for (u32 i = 0; i <= m_max_tag; i++)
		m_tag_active[i] = false;

	return 0;
}

static void m_destroy_tags()
{
	for (u32 i = 1; i <= m_max_tag; i++)
		SDL_free(m_tags[i].sides);
	SDL_free(m_tags);
	SDL_free(m_tag_active);
	m_max_tag = 0;
}

m_taglist* m_get_tags(u32 target)
{
	SDL_assert(target <= m_max_tag);
	return &m_tags[target];
}

static bool _m_flood_fill_cb_floor(m_cell* cell, void* data)
{
	u16 type = data;
	if (cell->floor.type == type)
		return false;
	cell->floor.type = type;
	return true;
}

static bool _m_flood_fill_cb_ceil(m_cell* cell, void* data)
{
	u16 type = data;
	if (cell->ceil.type == type)
		return false;
	cell->ceil.type = type;
	return true;
}

static bool _m_flood_fill_cb_brightness(m_cell* cell, void* data)
{
	u8 brightness = data;
	if (cell->brightness == brightness)
		return false;
	cell->brightness = brightness;
	return true;
}

static void _m_flood_fill(u16 x, u16 y, _m_flood_fill_action func, void* data)
{
	if (x >= m_map.w || y >= m_map.h)
		return;
	m_cell* cell = m_get_cell(x, y);
	if (!func(cell, data))
		return;

	if (!cell->e.type)
		_m_flood_fill(x + 1, y, func, data);
	if (!cell->n.type)
		_m_flood_fill(x, y - 1, func, data);
	if (!cell->w.type)
		_m_flood_fill(x - 1, y, func, data);
	if (!cell->s.type)
		_m_flood_fill(x, y + 1, func, data);
}

// Sets the given side and the one on the oppsoite side
static void _m_set_double_sided(u16 x, u16 y, m_orientation orientation, const m_side* side)
{
	SDL_memcpy(m_get_side(x, y, orientation), side, sizeof(m_side));
	SDL_memcpy(m_get_opposite_side(x, y, orientation), side, sizeof(m_side));
}
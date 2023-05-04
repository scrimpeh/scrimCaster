#include <map.h>

#include <actor.h>
#include <mapupdate.h>
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

Map m_map;

const char* tx_sets[] = { "test.png" };

extern ActorVector levelEnemies;

Cell cells[16][16];

u32 m_max_tag;
m_taglist* m_tags;
bool* m_tag_active;

Cell* m_get_cell(u16 x, u16 y)
{
	SDL_assert(x < m_map.w && y < m_map.h);
	return &m_map.cells[m_map.w * y + x];
}

Side* m_get_side(u16 x, u16 y, m_orientation o)
{
	return m_cell_get_side(m_get_cell(x, y), o);
}

void m_load()
{
	u8 i, j, n;
	u32 target = 1;
	m_map.w = 16;
	m_map.h = 16;
	m_map.cells = cells[0];
	m_map.info.texture_set_count = 1;
	m_map.info.texture_sets = tx_sets;
	m_map.info.sky = 0;

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
		cells[2][4].w.flags = (m_side_flags)(MIRR_H | SCROLL_H);
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
			Side side = { 0 };
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
		cells[7][1].n.door.openspeed = 1;
		cells[7][1].n.door.staytime = 12;
		cells[7][1].n.door.closespeed = 1;
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
			Side side = { 0 };
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
		_m_flood_fill(0, 0, _m_flood_fill_cb_brightness, 224);

		{
			Side side = { 0 };
			side.type = 5;
			side.flags = PASSABLE | TRANSLUCENT;
			_m_set_double_sided(4, 6, M_SOUTH, &side);
				
		}

		cells[0][6].flat.floor_type = 3;
		cells[0][6].flat.ceil_type = 3;

		_m_flood_fill(8, 2, _m_flood_fill_cb_ceil, 0);
		_m_flood_fill(8, 2, _m_flood_fill_cb_floor, 7);

		_m_flood_fill(4, 7, _m_flood_fill_cb_ceil, 0);
		_m_flood_fill(4, 7, _m_flood_fill_cb_brightness, 255);
		cells[5][6].brightness = 192;
		cells[6][6].brightness = 160;
		cells[6][5].brightness = 192;
	}

	Actor pil;
	pil.type = PILLAR;

	ActorArray* al = &m_map.levelObjs;
	ActorArrayMake(al, 5);
	
	pil.x = 300;
	pil.y = 100;
	al->actor[0] = pil;

	pil.x = 96;
	pil.y = 480;
	al->actor[1] = pil;
	
	pil.x = 448;
	pil.y = 320;
	al->actor[2] = pil;
	
	pil.x = 448;
	pil.y = 256;
	al->actor[3] = pil;
	
	pil.x = 448;
	pil.y = 192;
	al->actor[4] = pil;

	ActorVectorMake(&levelEnemies, 32);
	Actor* z = SDL_malloc(sizeof(Actor));
	SDL_memset(z, 0, sizeof(Actor));
	z->x = 300;
	z->y = 200;
	z->speed = 0.5;
	z->angle = 90;
	z->type = DUMMY_ENEMY;
	//ActorVectorAdd(&levelEnemies, z);

	z = SDL_malloc(sizeof(Actor));
	SDL_memset(z, 0, sizeof(Actor));
	z->x = 400;
	z->y = 200;
	z->speed = 0.5;
	z->angle = 90;
	z->type = DUMMY_ENEMY;
	//ActorVectorAdd(&levelEnemies, z);
	
	z = SDL_malloc(sizeof(Actor));
	SDL_memset(z, 0, sizeof(Actor));
	z->x = 8;
	z->y = 200;
	z->speed = 0.5;
	z->angle = 90;
	z->type = DUMMY_ENEMY;
	//ActorVectorAdd(&levelEnemies, z);

	//cells = SDL_malloc(sizeof(Cell) * 32 * 32);
	//SDL_assert(cells);
	m_create_tags();
};

void m_unload()
{
	ActorArrayDestroy(&m_map.levelObjs);
	ActorVectorDestroy(&levelEnemies);
	mu_clear();
	m_destroy_tags();
	tx_unload();
	//SDL_free(cells);
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
		if (!(m_tags[i].sides = SDL_malloc(sizeof(Side*) * m_tags[i].count)))
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
				Side* side = m_get_side(x, y, orientation);
				if (side->tag)
				{
					u32 next_tag = side_indices[side->tag]++;
					m_tags[side->tag].sides[next_tag] = side;
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

static bool _m_flood_fill_cb_floor(Cell* cell, void* data)
{
	u16 type = data;
	if (cell->flat.floor_type == type)
		return false;
	cell->flat.floor_type = type;
	return true;
}

static bool _m_flood_fill_cb_ceil(Cell* cell, void* data)
{
	u16 type = data;
	if (cell->flat.ceil_type == type)
		return false;
	cell->flat.ceil_type = type;
	return true;
}

static bool _m_flood_fill_cb_brightness(Cell* cell, void* data)
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
	Cell* cell = m_get_cell(x, y);
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
static void _m_set_double_sided(u16 x, u16 y, m_orientation orientation, const Side* side)
{
	m_orientation opposite;
	u16 opposite_x = x;
	u16 opposite_y = y;
	switch (orientation)
	{
	case M_EAST:
		opposite = M_WEST;
		opposite_x++;
		break;
	case M_NORTH:
		opposite = M_SOUTH;
		opposite_y--;
		break;
	case M_WEST:
		opposite = M_EAST;
		opposite_x--;
		break;
	case M_SOUTH:
		opposite = M_NORTH;
		opposite_y++;
		break;
	}
	SDL_memcpy(m_get_side(x, y, orientation), side, sizeof(Side));
	SDL_memcpy(m_get_side(opposite_x, opposite_y, opposite), side, sizeof(Side));
}
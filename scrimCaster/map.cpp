#include "map.h"

#include "actor.h"
#include "gfxloader.h"
#include "mapupdate.h"

#include "SDL/SDL_log.h"
#include "SDL/SDL_assert.h"

/* On scrolling doors:
 * When activated, a door switches from 'SOLID' into 'TRANSLUCENT' as well as other flags that need changing.
 * param1 shall define the scrollspeed (in terms of ticks per val)
 * while param2 shall define how long the door remains open (if at all), with all 0 meaning "forever"
 * we probably also need some status flags on the door to be shoehorned in
 *
 * scroll shall define "how open" the door is, i.e. it determines the ratio between open and not open
 */

bool mapLoaded;
Map map;
Cell* cellptr = NULL;

const char* tx = "test.png";
const char* tx_sets[] = { tx };
const char* none = "";

extern ActorVector levelEnemies;

Cell cellgrid[16][16];

Cell* GetCell(GridPos gp)
{
	SDL_assert(gp.x >= 0 && gp.x < map.boundsX);
	SDL_assert(gp.y >= 0 && gp.y < map.boundsY);

	return &map.cells[gp.y * map.boundsX + gp.x];
}

Cell* GetCell(double x, double y)
{
	const i16 offset_x = (i16)SDL_floor(x / CELLSIZE);
	const i16 offset_y = (i16)SDL_floor(y / CELLSIZE);

	return &map.cells[offset_y*map.boundsX + offset_x];
}

GridPos GetGridPosition(double x, double y)
{
	return { 
		(i16)SDL_floor(x / CELLSIZE), 
		(i16)SDL_floor(y / CELLSIZE) 
	};
}

u32 AsMapOffset(i16 x, i16 y, Orientation o)
{
	SDL_assert(o < 4);
	return ((y * map.boundsX) + x) * 4 + o;
};

Side* FromMapOffset(u32 offset)
{
	return &map.cells[offset / 4].sides[offset % 4];
};

void LoadMap()
{
	u8 i, j, n;
	map.boundsX = 16;
	map.boundsY = 16;
	map.cells = cellgrid[0];
	map.info.txSetCount = 1;
	map.info.txSets = tx_sets;
	map.info.floorname = none;
	map.info.skyname = none;
	map.info.mus = none;

	SDL_assert(map.info.txSetCount > 0);

	for (u32 i = 0; i < map.info.txSetCount; ++i)
	{
		LoadMapTexture(map.info.txSets[i]);
	}

	for (i = 0; i < 8; ++i)
		for (j = 0; j < 8; ++j)
			for (n = 0; n < 4; ++n)
				cellgrid[i][j].sides[n].type = 0;

	//make rudimentary walls
	{
		for (u8 i = 0; i < 7; ++i)
		{
			cellgrid[0][i].n.type = 1;
			cellgrid[6][i].s.type = 1;
			cellgrid[i][0].w.type = 1;
		}
		cellgrid[4][0].w.type = 0;

		cellgrid[0][6].e.type = 3;
		cellgrid[2][6].e.type = 1;
		cellgrid[6][6].e.type = 3;
		cellgrid[5][6].e.type = 1;
		cellgrid[2][7].w.type = 1;
		cellgrid[5][7].w.type = 1;
		for (u8 i = 0; i < 4; ++i)
			cellgrid[2 + i][9].e.type = 1;
		for (u8 i = 0; i < 3; ++i)
		{
			cellgrid[2][7 + i].n.type = 1;
			cellgrid[5][7 + i].s.type = 1;
		}
		cellgrid[3][9].e.type = 5;
		cellgrid[4][9].e.type = 5;
		for (u8 i = 0; i < 2; ++i)
		{
			cellgrid[3 + i][6].e.type = 4;
			cellgrid[3 + i][6].e.flags = SideFlags(PASSABLE | TRANSLUCENT);
			cellgrid[3 + i][7].w.type = 4;
			cellgrid[3 + i][7].w.flags = SideFlags(PASSABLE | TRANSLUCENT);
		}


		cellgrid[1][1].e.type = 3;
		cellgrid[1][1].s.type = 1;
		cellgrid[1][1].s.flags = SCROLL_H;
		cellgrid[1][1].s.param1 = 8;
		cellgrid[0][2].s.type = 5;
		cellgrid[1][3].s.type = 1;
		cellgrid[1][3].w.type = 1;
		cellgrid[2][4].w.type = 5;
		cellgrid[2][4].w.flags = SideFlags(MIRR_H | SCROLL_H);
		cellgrid[2][4].w.param1 = -8;
		cellgrid[2][0].e.type = 5;
		cellgrid[3][1].e.type = 1;
		cellgrid[3][1].n.type = 1;
		cellgrid[3][3].w.type = 1;
		cellgrid[3][3].n.type = 1;
		cellgrid[4][2].n.type = 5;
		cellgrid[0][6].s.type = 1;
		cellgrid[1][5].e.type = 3;
		cellgrid[2][6].n.type = 1;

		cellgrid[6][1].s.type = 0;
		cellgrid[6][4].s.type = 0;
		cellgrid[7][1].e.type = 3;
		cellgrid[7][1].w.type = 1;
		cellgrid[8][1].w.type = 1;
		cellgrid[8][1].s.type = 1;
		cellgrid[8][2].n.type = 1;
		cellgrid[8][2].s.type = 1;
		cellgrid[8][3].n.type = 1;
		cellgrid[8][3].s.type = 3;
		cellgrid[8][4].s.type = 3;
		cellgrid[8][4].e.type = 1;
		cellgrid[7][4].e.type = 1;
		cellgrid[7][4].w.type = 3;

		cellgrid[4][2].w.type = 2;
		cellgrid[5][2].w.type = 2;
		cellgrid[6][2].w.type = 2;
		cellgrid[4][1].e.type = 2;
		cellgrid[5][1].e.type = 2;
		cellgrid[6][1].e.type = 2;
		cellgrid[4][2].w.flags = TRANSLUCENT;
		cellgrid[5][2].w.flags = TRANSLUCENT;
		cellgrid[6][2].w.flags = TRANSLUCENT;
		cellgrid[4][1].e.flags = TRANSLUCENT;
		cellgrid[5][1].e.flags = TRANSLUCENT;
		cellgrid[6][1].e.flags = TRANSLUCENT;

		cellgrid[6][1].s.type = 5;
		cellgrid[6][1].s.flags = DOOR_V;
		cellgrid[6][1].s.door.status = 0;
		cellgrid[6][1].s.door.openspeed = 12;
		cellgrid[6][1].s.door.staytime = 12;
		cellgrid[6][1].s.door.closespeed = 12;
		cellgrid[6][1].s.door.door_flags = DoorFlags(LINKED | PLAYER_ACTIVATE); //double sided
		cellgrid[6][1].s.door.linked_to = AsMapOffset(1, 7, NORTH);

		cellgrid[7][1].n.type = 5;
		cellgrid[7][1].n.flags = DOOR_V;
		cellgrid[7][1].n.door.status = 0;
		cellgrid[7][1].n.door.openspeed = 1;
		cellgrid[7][1].n.door.staytime = 12;
		cellgrid[7][1].n.door.closespeed = 1;
		cellgrid[7][1].n.door.door_flags = DoorFlags(LINKED | PLAYER_ACTIVATE); //double sided
		cellgrid[7][1].n.door.linked_to = AsMapOffset(1, 6, SOUTH);

	}

	Actor pil;
	pil.type = PILLAR;

	auto al = &map.levelObjs;
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
	Actor* z = (Actor*)SDL_malloc(sizeof(Actor));
	SDL_memset(z, 0, sizeof(Actor));
	z->x = 300;
	z->y = 200;
	z->speed = 0.5;
	z->angle = 90;
	z->type = DUMMY_ENEMY;
	//ActorVectorAdd(&levelEnemies, z);

	z = (Actor*)SDL_malloc(sizeof(Actor));
	SDL_memset(z, 0, sizeof(Actor));
	z->x = 400;
	z->y = 200;
	z->speed = 0.5;
	z->angle = 90;
	z->type = DUMMY_ENEMY;
	//ActorVectorAdd(&levelEnemies, z);
	
	z = (Actor*)SDL_malloc(sizeof(Actor));
	SDL_memset(z, 0, sizeof(Actor));
	z->x = 8;
	z->y = 200;
	z->speed = 0.5;
	z->angle = 90;
	z->type = DUMMY_ENEMY;
	//ActorVectorAdd(&levelEnemies, z);

	//cellptr = (Cell*)SDL_malloc(sizeof(Cell) * 32 * 32);
	//SDL_assert(cellptr);
};

void UnloadMap()
{
	ActorArrayDestroy(&map.levelObjs);
	ActorVectorDestroy(&levelEnemies);
	ClearActiveSides();
	//SDL_free(cellptr);
};
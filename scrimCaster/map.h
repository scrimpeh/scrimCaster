#pragma once

#include <common.h>

#include <cell.h>
#include <actorcontainers.h>

typedef struct MapInfo
{
	char name[32];
	u8 txSetCount;	//Textures
	const char** txSets;
	const char* skyname;
} MapInfo;

typedef struct Map
{
	MapInfo info;
	u16 w;
	u16 h;
	Cell* cells;	//A pointer to a cell structure contained somewhere on the heap
					//should contain exactly bounds X * bounds Y cells
	ActorArray levelObjs;
	ActorArray levelEnemies;
	ActorList levelPickups;
} Map;

typedef struct
{
	Side** sides;
	u32 count;
} m_taglist;

void m_load();
void m_unload();

extern Map m_map;

Cell* m_get_cell(u16 x, u16 y);
Side* m_get_side(u16 x, u16 y, m_orientation o);
m_taglist* m_get_tags(u32 target);

static i32 m_create_tags();
static void m_destroy_tags();

// Development function, flood-fills a floor with a certain type
static void _m_flood_fill(u16 x, u16 y, u16 type, bool floor);
static void _m_flood_fill_inner(u16 x, u16 y, u16 type, u16 initial, bool floor);
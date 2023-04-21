#pragma once

#include "common.h"

#include "cell.h"
#include "actorcontainers.h"

typedef struct GridPos
{
	i16 x; i16 y;
} GridPos;

typedef struct MapInfo
{
	char mapName[32];
	u32 mapIndex;
	u8 txSetCount;	//Textures
	const char** txSets;
	const char* skyname;
	const char* floorname;
	const char* mus;
} MapInfo;

typedef struct Map
{
	MapInfo info;
	u16 boundsX;
	u16 boundsY;
	Cell* cells;	//A pointer to a cell structure contained somewhere on the heap
					//should contain exactly bounds X * bounds Y cells
	ActorArray levelObjs;
	ActorArray levelEnemies;
	ActorList levelPickups;
} Map;

typedef enum Orientation : u8
{
	EAST = 0,
	NORTH = 1,
	WEST = 2,
	SOUTH  = 3
} Orientation;

Cell* GetCell(double x, double y);
Cell* GetCell(GridPos gp);
GridPos GetGridPosition(double x, double y);

u32 AsMapOffset(i16 x, i16 y, Orientation o);
Side* FromMapOffset(u32 offset);

void LoadMap();
void UnloadMap();
#pragma once

#include "map.h"

typedef struct Coordinates
{
	double x, y;
} Coordinates;

typedef struct SideCoords
{
	i16 grid_x, grid_y;
	double p_x, p_y;
	Side* side;
} Sidecoords;

Coordinates ProjectVector(Coordinates in, double angle, double disp);
Coordinates ProjectVector(double x_in, double y_in, double angle, double disp);
bool IntersectWall(SideCoords* coords, Coordinates origin, Coordinates dest);
bool IntersectWall(SideCoords* coords, Coordinates origin, double angle);

bool IntersectWall(SideCoords* coords, Coordinates origin, Coordinates dest, bool(*hit)(const Side*));
bool IntersectWall(SideCoords* coords, Coordinates origin, double angle, bool(*hit)(const Side*));

//Comparison functions for map intersections
bool IsSide(const Side* s);
bool HitscanSolid(const Side* s);
bool VisibilitySolid(const Side* s);
#include "types.h"
#include "maputil.h"
#include "renderconstants.h"

#include "SDL/SDL_assert.h"
#include "SDL/SDL_log.h"
#include "math.h"


const float MAXSLOPE = 1e+8f;
extern Map map;

Coordinates ProjectVector(Coordinates in, double angle, double disp)
{
	return ProjectVector(in.x, in.y, angle, disp);
}

Coordinates ProjectVector(double x_in, double y_in, double angle, double disp)
{
	const double arctan = TO_RAD(ATAN_DEG(angle));

	const double d_x = disp * SDL_cos(arctan);		//Calculate forward momentum
	const double d_y = disp * SDL_sin(arctan) * -1;

	return { x_in + d_x, y_in + d_y };
}

//Determines if, along a given line, a wall has been hit
bool IntersectWall(SideCoords* coords, Coordinates origin, Coordinates dest)
{
	return IntersectWall(coords, origin, dest, IsSide);
}

bool IntersectWall(SideCoords* coords, Coordinates origin, double angle)
{
	return IntersectWall(coords, origin, angle, IsSide);
}

bool IntersectWall(SideCoords* coords, Coordinates origin, Coordinates dest, bool (*hit)(const Side*))
{
	Side* s;
	
	const double angle = AngleToDeg(origin.x, dest.y, dest.x, origin.y);
	const bool north = angle < 180;
	const bool east = angle < 90 || angle > 270;
	double slope = -1 * tan(TO_RAD(angle));
	if (fabs(slope) > MAXSLOPE)
		slope = slope > 0 ? MAXSLOPE : -MAXSLOPE;

	i16 grid_x = (i16)floor(origin.x / CELLSIZE);
	i16 grid_y = (i16)floor(origin.y / CELLSIZE);

	const i16 end_x = (i16)floor(dest.x / CELLSIZE);
	const i16 end_y = (i16)floor(dest.y / CELLSIZE);
	double p_x = origin.x, p_y = origin.y;

	const i8 x_inc = east ? 1 : -1;
	const i8 y_inc = north ? -1 : 1;

	double diff_x = (!east ? grid_x : grid_x + 1)*CELLSIZE - p_x;
trace:
	p_x += diff_x;
	p_y += diff_x * slope;

	const i16 new_grid_y = (i16)floor(p_y / CELLSIZE);
	while (grid_y != new_grid_y)
	{
		if (north ? grid_y < SDL_max(0, end_y + 1) : grid_y > SDL_min(map.boundsY, end_y - 1))
			goto end;

		Cell* c = GetCell( { grid_x, grid_y } );
		s = north ? &c->n : &c->s;
		if (hit(s))
		{
			p_x -= diff_x;
			p_y -= diff_x * slope;

			const i16 grid_y_side = north ? grid_y : grid_y + 1;

			p_x += ((grid_y_side * CELLSIZE) - p_y) / slope;
			p_y = double(grid_y_side * CELLSIZE);

			goto hit_wall;
		}
			
		grid_y += y_inc;
	}

	if (east ? grid_x > SDL_min(map.boundsX - 1, end_x - 1) : grid_x < SDL_max(0, end_x + 1))
		goto end;

	Cell* c = GetCell({ grid_x, grid_y });
	s = east ? &c->e : &c->w;
	if (hit(s))
	{
		goto hit_wall;
	}

	diff_x = east ? CELLSIZE : -CELLSIZE;
	grid_x += x_inc;
	goto trace;

end:
	return false;

hit_wall:
	coords->grid_x = grid_x;
	coords->grid_y = grid_y;
	coords->p_x = p_x;
	coords->p_y = p_y;
	coords->side = s;
	return true;
}

bool IntersectWall(SideCoords* coords, Coordinates origin, double angle, bool (*hit)(const Side*))
{
	Side* s;

	const bool north = angle < 180;
	const bool east = angle < 90 || angle > 270;
	double slope = -1 * tan(TO_RAD(angle));
	if (fabs(slope) > MAXSLOPE)
		slope = slope > 0 ? MAXSLOPE : -MAXSLOPE;

	i16 grid_x = (i16)floor(origin.x / CELLSIZE);
	i16 grid_y = (i16)floor(origin.y / CELLSIZE);

	double p_x = origin.x, p_y = origin.y;

	const i8 x_inc = east ? 1 : -1;
	const i8 y_inc = north ? -1 : 1;

	double diff_x = (!east ? grid_x : grid_x + 1)*CELLSIZE - p_x;
trace:
	p_x += diff_x;
	p_y += diff_x * slope;

	const i16 new_grid_y = (i16)floor(p_y / CELLSIZE);
	while (grid_y != new_grid_y)
	{
		if (north ? grid_y < 0 : grid_y > map.boundsY)
			goto end;

		Cell* c = GetCell({ grid_x, grid_y });
		s = north ? &c->n : &c->s;
		if (hit(s))
		{
			p_x -= diff_x;
			p_y -= diff_x * slope;

			const i16 grid_y_side = north ? grid_y : grid_y + 1;

			p_x += ((grid_y_side * CELLSIZE) - p_y) / slope;
			p_y = double(grid_y_side * CELLSIZE);

			goto hit_wall;
		}

		grid_y += y_inc;
	}

	if (east ? grid_x > map.boundsX - 1 : grid_x < 0)
		goto end;

	Cell* c = GetCell({ grid_x, grid_y });
	s = east ? &c->e : &c->w;
	if (hit(s))
	{
		goto hit_wall;
	}

	diff_x = east ? CELLSIZE : -CELLSIZE;
	grid_x += x_inc;
	goto trace;

end:
	return false;

hit_wall:
	coords->grid_x = grid_x;
	coords->grid_y = grid_y;
	coords->p_x = p_x;
	coords->p_y = p_y;
	coords->side = s;
	return true;
}

bool IsSide(const Side* s)
{
	return s->type != 0;
}

bool HitscanSolid(const Side* s)
{
	return s->type != 0 && !(s->flags & BULLETS_PASS) &&
		(s->flags & DOOR_V ? s->door.scroll < 32 : true);
}

bool VisibilitySolid(const Side* s)
{
	return s->type != 0 && !(s->flags & TRANSLUCENT);
}
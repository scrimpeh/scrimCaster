#include <game/actor/actor.h>

#include <game/actor/actorcontainers.h>
#include <game/gameobjects.h>
#include <map/map.h>
#include <render/renderconstants.h>

#define AC_MOVE_COLLIDE_WALL  1
#define AC_MOVE_COLLIDE_ACTOR 2

const double MIN_WALL_DIST = 1e-12;

extern m_map_data m_map;

static const ac_bounds AC_BOUNDS[] =
{
	[AC_DUMMY]       = { 0., 0. },
	[AC_PLAYER]      = { 8., 8. },
	[AC_DUMMY_ENEMY] = { 8., 8. },
	[AC_PILLAR]      = { 8., 8. }
};

ac_bounds ac_get_bounds(ac_type type)
{
	return AC_BOUNDS[type];
}

static bool ac_collide_h(const ac_actor* actor, double* p_dx)
{
	double d_x = *p_dx;

	if (d_x == 0) 
		return false;

	// Draw a line on the actor's movement plane check if intersected a wall
	const ac_bounds cur_bounds = ac_get_bounds(actor->type);
	const double bounds_x = cur_bounds.w * (d_x >= 0 ? 1 : -1);
	const i16 x_start = (i16) SDL_floor((actor->x - bounds_x) / M_CELLSIZE);
	const i16 x_end = (i16) SDL_floor((actor->x + d_x + bounds_x) / M_CELLSIZE);
	const i16 y_top = (i16) SDL_floor((actor->y - cur_bounds.h) / M_CELLSIZE);
	const i16 y_bottom = (i16) SDL_floor((actor->y + cur_bounds.h) / M_CELLSIZE);

	const m_cell* cells = m_map.cells;

	const i8 inc = x_start > x_end ? -1 : 1;
	for (i16 x = x_start; x != x_end; x += inc)
	{
		const m_cell cell_top = cells[y_top * m_map.w + x];
		const m_cell cell_bottom = cells[y_bottom * m_map.w + x];

		const m_side side_top = d_x > 0 ? cell_top.e : cell_top.w;
		const m_side side_bottom = d_x > 0 ? cell_bottom.e : cell_bottom.w;

		if ((side_top.type && !(side_top.flags  & PASSABLE)) ||
			(side_bottom.type && !(side_bottom.flags & PASSABLE)))
		{
			double new_x = (double) (x * M_CELLSIZE) - bounds_x;
			if (d_x >= 0) 
				new_x += (double) (M_CELLSIZE) - MIN_WALL_DIST;
			*p_dx = new_x - actor->x;
			return true;
		}
	}

	return false;
}

static bool ac_collide_v(const ac_actor* actor, double* p_dy)
{
	double d_y = *p_dy;

	if (d_y == 0) 
		return false;

	// Draw a line on the actor's movement plane check if intersected a wall
	const ac_bounds cur_bounds = ac_get_bounds(actor->type);
	const double bounds_y = cur_bounds.h * (d_y >= 0 ? 1 : -1);
	const i16 y_start = (i16) SDL_floor((actor->y - bounds_y) / M_CELLSIZE);
	const i16 y_end = (i16) SDL_floor((actor->y + d_y + bounds_y) / M_CELLSIZE);
	const i16 x_left = (i16) SDL_floor((actor->x - cur_bounds.w) / M_CELLSIZE);
	const i16 x_right = (i16) SDL_floor((actor->x + cur_bounds.w) / M_CELLSIZE);

	const m_cell* cells = m_map.cells;

	const i8 inc = y_start > y_end ? -1 : 1;
	for (i16 y = y_start; y != y_end; y += inc)
	{
		const m_cell cell_left = cells[y * m_map.w + x_left];
		const m_cell cell_right = cells[y * m_map.w + x_right];

		const m_side side_left = d_y > 0 ? cell_left.s : cell_left.n;
		const m_side side_right = d_y > 0 ? cell_right.s : cell_right.n;

		if ((side_left.type  && !(side_left.flags  & PASSABLE)) ||
			(side_right.type && !(side_right.flags & PASSABLE)))
		{
			double new_y = (double) (y * M_CELLSIZE) - bounds_y;
			if (d_y >= 0) 
				new_y += (double) (M_CELLSIZE) - MIN_WALL_DIST;
			*p_dy = new_y - actor->y;
			return true;
		}
	}

	return false;
}

static bool ac_intersect(ac_actor* actor, const ac_actor* obstacle, bool vertical, double* disp)
{
	if (actor == obstacle) 
		return false;

	const ac_bounds cur_bounds = ac_get_bounds(actor->type);
	const ac_bounds obstacle_bounds = ac_get_bounds(obstacle->type);

	const double d_xy = *disp;
	double a_right, a_top, a_left, a_bottom;
	double b_right, b_top, b_left, b_bottom;

	if (vertical)
	{
		a_right =  actor->x        + cur_bounds.w;
		a_left =   actor->x        - cur_bounds.w;
		a_bottom = actor->y + d_xy + cur_bounds.h;
		a_top =    actor->y + d_xy - cur_bounds.h;
	}
	else
	{
		a_right =  actor->x + d_xy + cur_bounds.w;
		a_left =   actor->x + d_xy - cur_bounds.w;
		a_bottom = actor->y        + cur_bounds.h;
		a_top =    actor->y        - cur_bounds.h;
	}

	b_right  = obstacle->x + obstacle_bounds.w;
	b_left   = obstacle->x - obstacle_bounds.w;
	b_bottom = obstacle->y + obstacle_bounds.h;
	b_top    = obstacle->y - obstacle_bounds.h;

	if (a_right < b_left || a_left > b_right || a_bottom < b_top || a_top > b_bottom)
		return false;

	double new_pos;
	if (vertical)
	{
		if (d_xy >= 0)
			new_pos = obstacle->y - obstacle_bounds.h - cur_bounds.h - MIN_WALL_DIST;
		else
			new_pos = obstacle->y + obstacle_bounds.h + cur_bounds.h + MIN_WALL_DIST;
		*disp = new_pos - actor->y;
	}
	else
	{
		if (d_xy >= 0)
			new_pos = obstacle->x - obstacle_bounds.w - cur_bounds.w - MIN_WALL_DIST;
		else
			new_pos = obstacle->x + obstacle_bounds.w + cur_bounds.w + MIN_WALL_DIST;
		*disp = new_pos - actor->x;
	}

	return true;
}

static bool ac_collide_actor(ac_actor* actor, bool vertical, double* disp)
{
	bool collision = false;
	const ac_list_node* node = ac_actors.first;
	while (node)
	{
		const ac_actor* cur = &node->actor;
		collision |= ac_intersect(actor, cur, vertical, disp);
		node = node->next;
	}

	return collision;
}

// Returns 'true', if a collision occured, false otherwise
bool ac_move(ac_actor* actor, u32 delta, u32 flags)
{
	actor->angle = angle_normalize_deg_d(actor->angle);

	bool collision = false;
	double arctan, d_x, d_y;
	arctan = TO_RAD(ATAN_DEG(actor->angle));

	const double cos_atan = SDL_cos(arctan);
	const double sin_atan = SDL_sin(arctan);

	d_x = actor->speed * cos_atan;
	d_y = actor->speed * sin_atan * -1;
	d_x += actor->strafe * sin_atan * -1;
	d_y += actor->strafe * cos_atan * -1;

	if (flags & AC_MOVE_COLLIDE_ACTOR)
		collision |= ac_collide_actor(actor, true, &d_y);
	if (flags & AC_MOVE_COLLIDE_WALL)
		collision |= ac_collide_v(actor, &d_y);
	actor->y += d_y;

	if (flags & AC_MOVE_COLLIDE_ACTOR)
		collision |= ac_collide_actor(actor, false, &d_x);
	if (flags & AC_MOVE_COLLIDE_WALL)
		collision |= ac_collide_h(actor, &d_x);
	actor->x += d_x;

	return collision;
}

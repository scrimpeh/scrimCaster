#include <automap.h>

#include <camera.h>
#include <enemy.h>
#include <map.h>
#include <player.h>
#include <render.h>
#include <renderutil.h>


// Cap the automap to the bounds of the map and inside this zone.
#define AM_MARIGN (2 * M_CELLSIZE)

// If am_foölow is false, center the map around the this point, otherwise,
// center the map around the player
float am_zoom = 1.25;
float am_center_x = 0.f;
float am_center_y = 0.f;
bool am_follow = true;

static float am_cell_size;
static float am_x;
static float am_y;

static float r_map_intercept_x;
static float r_map_intercept_y;

void am_init()
{
	am_center_x = (float)m_map.w * M_CELLSIZE / 2;
	am_center_y = (float)m_map.h * M_CELLSIZE / 2;
}

void am_draw(SDL_Surface* target)
{
	am_cell_size = (1.0 / 16) * am_zoom;

	am_x = am_follow ? player.x : am_center_x;
	am_y = am_follow ? player.y : am_center_y;

	const float viewport_ratio = (float)viewport_w / viewport_h;
	const float am_x_min = AM_MARIGN * viewport_ratio;
	const float am_x_max = (m_map.w * M_CELLSIZE - AM_MARIGN) * viewport_ratio;
	const float am_y_min = AM_MARIGN;
	const float am_y_max = m_map.h * M_CELLSIZE - AM_MARIGN;

	am_x = SDL_min(SDL_max(am_x, am_x_min), am_x_max);
	am_y = SDL_min(SDL_max(am_y, am_y_min), am_y_max);

	am_draw_map(target);
	am_draw_actors(target);
}

static void am_draw_cell(SDL_Surface* target, i16 grid_x, i16 grid_y)
{
	const Cell* cell = m_map.cells + grid_y * m_map.h + grid_x;
	am_draw_side(target, grid_x, grid_y, &cell->e, SIDE_ORIENTATION_EAST);
	am_draw_side(target, grid_x, grid_y, &cell->n, SIDE_ORIENTATION_NORTH);
	am_draw_side(target, grid_x, grid_y, &cell->w, SIDE_ORIENTATION_WEST);
	am_draw_side(target, grid_x, grid_y, &cell->s, SIDE_ORIENTATION_SOUTH);
}

static i32 am_map_h(float x)
{
	float rel_grid = (x - am_x) / M_CELLSIZE;
	return r_hud_px_h(rel_grid * am_cell_size) + r_hud_px_h(.5f);
}

static i32 am_map_v(float y)
{
	float rel_grid = (y - am_y) / M_CELLSIZE;
	return r_hud_px_h(rel_grid * am_cell_size) + r_hud_px_v(.5f);
}

static i32 am_map_distance(float d)
{
	float rel_size = am_cell_size / M_CELLSIZE;
	return r_hud_px_h(d * rel_size);
}

static void am_draw_side(SDL_Surface* target, i16 grid_x, i16 grid_y, const Side* side, g_side_orientation orientation)
{
	if (!side->type)
		return;

	const i32 px_x = am_map_h(grid_x * M_CELLSIZE);
	const i32 px_y = am_map_v(grid_y * M_CELLSIZE);
	const i32 length = am_map_distance(M_CELLSIZE);

	i32 x_a;
	i32 y_a;
	i32 x_b;
	i32 y_b;
	switch (orientation)
	{
	case SIDE_ORIENTATION_EAST:
		x_a = px_x + length;
		y_a = px_y;
		x_b = px_x + length;
		y_b = px_y + length;
		break;
	case SIDE_ORIENTATION_NORTH:
		x_a = px_x;
		y_a = px_y;
		x_b = px_x + length;
		y_b = px_y;
		break;
	case SIDE_ORIENTATION_WEST:
		x_a = px_x;
		y_a = px_y;
		x_b = px_x;
		y_b = px_y + length;
		break;
	case SIDE_ORIENTATION_SOUTH:
		x_a = px_x;
		y_a = px_y + length;
		x_b = px_x + length;
		y_b = px_y + length;
		break;
	}

	// TODO: More advanced side rendering
	r_draw_line(target, x_a, y_a, x_b, y_b, CM_GET(255, 255, 128));
}


static void am_draw_map(SDL_Surface* target)
{
	for (u16 y = 0; y < m_map.h; y++)
		for (u16 x = 0; x < m_map.w; x++)
			am_draw_cell(target, x, y);
}

void am_draw_actor(SDL_Surface* target, Actor* actor)
{
	// Cast two rays showing FOV for the player
	if (actor->type == PLAYER)
	{
		const angle_d angle_l = angle_normalize_deg_d(player.angle + (viewport_x_fov / 2.));
		const angle_d angle_r = angle_normalize_deg_d(player.angle - (viewport_x_fov / 2.));

		double intersect_l_x;
		double intersect_l_y;
		g_cast(player.x, player.y, TO_RADF(angle_l), am_collect_intercept);
		intersect_l_x = am_map_h(r_map_intercept_x);
		intersect_l_y = am_map_v(r_map_intercept_y);

		double intersect_r_x;
		double intersect_r_y;
		g_cast(player.x, player.y, TO_RADF(angle_r), am_collect_intercept);
		intersect_r_x = am_map_h(r_map_intercept_x);
		intersect_r_y = am_map_v(r_map_intercept_y);

		r_draw_line(target, am_map_h(actor->x), am_map_v(actor->y), intersect_l_x, intersect_l_y, CM_GET(0xFF, 0xFF, 0xFF));
		r_draw_line(target, am_map_h(actor->x), am_map_v(actor->y), intersect_r_x, intersect_r_y, CM_GET(0xFF, 0xFF, 0xFF));
	}

	const Bounds bounds = GetActorBounds(actor->type);
	SDL_Rect r;
	r.x = am_map_h(actor->x - bounds.x);
	r.y = am_map_v(actor->y - bounds.y);
	r.w = am_map_distance(bounds.x * 2);
	r.h = am_map_distance(bounds.y * 2);
	SDL_FillRect(target, &r, actor->type == PLAYER ? CM_GET(255, 255, 0) : CM_GET(255, 0, 255));
}

void am_draw_actors(SDL_Surface* target)
{
	am_draw_actor(target, &player);

	for (u32 i = 0; i < m_map.levelObjs.count; i++)
		am_draw_actor(target, &m_map.levelObjs.actor[i]);
	for (u32 i = 0; i < m_map.levelEnemies.count; i++)
		am_draw_actor(target, &m_map.levelEnemies.actor[i]);
}

static bool am_collect_intercept(const g_intercept* intercept)
{
	// This is not re-entrant, and I'd rather have a closure, but oh well
	r_map_intercept_x = intercept->x;
	r_map_intercept_y = intercept->y;

	return intercept->type == G_INTERCEPT_NON_SOLID;
}
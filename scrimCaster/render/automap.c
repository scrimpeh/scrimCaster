#include <render/automap.h>

#include <game/actor/actorcontainers.h>
#include <game/camera.h>
#include <game/gameobjects.h>
#include <input/input.h>
#include <map/map.h>
#include <render/render.h>
#include <render/renderutil.h>
#include <render/skybox.h>
#include <render/viewport.h>


// Cap the automap to the # of the map and inside this zone.
#define AM_MARIGN (2 * M_CELLSIZE)

// If am_foölow is false, center the map around the this point, otherwise,
// center the map around the player
float am_zoom = 1.25;
float am_center_x = 0.f;
float am_center_y = 0.f;
bool am_follow = true;
bool am_draw_grid = true;

static const float AM_ZOOM_MIN = 0.5;
static const float AM_ZOOM_MAX = 3;
static const float AM_ZOOM_INC = 0.25;

static float am_cell_size;
static float am_x;
static float am_y;

static float r_map_intercept_x;
static float r_map_intercept_y;

extern i64 input_mwheel;

void am_init()
{
	am_center_x = (float)m_map.w * M_CELLSIZE / 2;
	am_center_y = (float)m_map.h * M_CELLSIZE / 2;
}

void am_draw(SDL_Surface* target)
{
	// It's sorta ugly to process input at the same point while drawing,
	// but this is the most convenient place to do it
	am_zoom += AM_ZOOM_INC * input_mwheel;
	am_zoom = SDL_max(SDL_min(AM_ZOOM_MAX, am_zoom), AM_ZOOM_MIN);

	am_cell_size = (1.0 / 16) * am_zoom;

	const ac_actor* player = ac_get_player();
	am_x = am_follow ? player->x : am_center_x;
	am_y = am_follow ? player->y : am_center_y;

	const float viewport_ratio = (float) viewport_w / viewport_h;
	const float am_margin_zoom = AM_MARIGN / am_zoom;
	const float am_x_min = am_margin_zoom * viewport_ratio;
	const float am_x_max = (m_map.w * M_CELLSIZE - am_margin_zoom) * viewport_ratio;
	const float am_y_min = am_margin_zoom;
	const float am_y_max = m_map.h * M_CELLSIZE - am_margin_zoom;

	am_x = SDL_min(SDL_max(am_x, am_x_min), am_x_max);
	am_y = SDL_min(SDL_max(am_y, am_y_min), am_y_max);

	am_draw_map(target);
	am_draw_actors(target);
}

static void am_draw_cell(SDL_Surface* target, i16 grid_x, i16 grid_y)
{
	const m_cell* cell = m_map.cells + grid_y * m_map.h + grid_x;
	am_draw_side(target, grid_x, grid_y, &cell->e, M_EAST);
	am_draw_side(target, grid_x, grid_y, &cell->n, M_NORTH);
	am_draw_side(target, grid_x, grid_y, &cell->w, M_WEST);
	am_draw_side(target, grid_x, grid_y, &cell->s, M_SOUTH);
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

static void am_draw_side(SDL_Surface* target, i16 grid_x, i16 grid_y, const m_side* side, m_orientation orientation)
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
	case M_EAST:
		x_a = px_x + length;
		y_a = px_y;
		x_b = px_x + length;
		y_b = px_y + length;
		break;
	case M_NORTH:
		x_a = px_x;
		y_a = px_y;
		x_b = px_x + length;
		y_b = px_y;
		break;
	case M_WEST:
		x_a = px_x;
		y_a = px_y;
		x_b = px_x;
		y_b = px_y + length;
		break;
	case M_SOUTH:
		x_a = px_x;
		y_a = px_y + length;
		x_b = px_x + length;
		y_b = px_y + length;
		break;
	}

	r_draw_line(target, x_a, y_a, x_b, y_b, am_get_color(side));
}


static void am_draw_map(SDL_Surface* target)
{
	if (am_draw_grid)
	{
		for (u16 y = 0; y <= m_map.h; y++)
			r_draw_line(target, am_map_h(0 * M_CELLSIZE), am_map_v(y * M_CELLSIZE), am_map_h(m_map.w * M_CELLSIZE), am_map_v(y * M_CELLSIZE), CM_GET(128, 128, 128));
		for (u16 x = 0; x <= m_map.w; x++)
			r_draw_line(target, am_map_h(x * M_CELLSIZE), am_map_v(0 * M_CELLSIZE), am_map_h(x * M_CELLSIZE), am_map_v(m_map.h * M_CELLSIZE), CM_GET(96, 96, 96));
	}
	for (u16 y = 0; y < m_map.h; y++)
		for (u16 x = 0; x < m_map.w; x++)
			am_draw_cell(target, x, y);
}

void am_draw_actor(SDL_Surface* target, ac_actor* actor)
{
	// Cast two rays showing FOV for the player
	if (actor->type == AC_PLAYER)
	{
		const ac_actor* player = ac_get_player();
		const angle_d angle_l = angle_normalize_deg_d(player->angle + (viewport_x_fov / 2.));
		const angle_d angle_r = angle_normalize_deg_d(player->angle - (viewport_x_fov / 2.));

		double intersect_l_x;
		double intersect_l_y;
		g_cast(player->x, player->y, TO_RADF(angle_l), am_collect_intercept);
		intersect_l_x = am_map_h(r_map_intercept_x);
		intersect_l_y = am_map_v(r_map_intercept_y);

		double intersect_r_x;
		double intersect_r_y;
		g_cast(player->x, player->y, TO_RADF(angle_r), am_collect_intercept);
		intersect_r_x = am_map_h(r_map_intercept_x);
		intersect_r_y = am_map_v(r_map_intercept_y);

		r_draw_line(target, am_map_h(actor->x), am_map_v(actor->y), intersect_l_x, intersect_l_y, CM_GET(0xFF, 0xFF, 0xFF));
		r_draw_line(target, am_map_h(actor->x), am_map_v(actor->y), intersect_r_x, intersect_r_y, CM_GET(0xFF, 0xFF, 0xFF));
	}

	const ac_bounds bounds = ac_get_bounds(actor->type);
	SDL_Rect r;
	r.x = am_map_h(actor->x - bounds.w);
	r.y = am_map_v(actor->y - bounds.h);
	r.w = am_map_distance(bounds.w * 2);
	r.h = am_map_distance(bounds.h * 2);
	SDL_FillRect(target, &r, actor->type == AC_PLAYER ? CM_GET(255, 255, 0) : CM_GET(255, 0, 255));
}

void am_draw_actors(SDL_Surface* target)
{
	ac_list_node* cur = ac_actors.first;
	while (cur)
	{
		am_draw_actor(target, &cur->actor);
		cur = cur->next;
	}
}

static bool am_collect_intercept(const g_intercept* intercept)
{
	// This is not re-entrant, and I'd rather have a closure, but oh well
	r_map_intercept_x = intercept->wx;
	r_map_intercept_y = intercept->wy;
	return intercept->type == G_INTERCEPT_NON_SOLID;
}

static cm_color am_get_color(const m_side* side)
{
	if (side->type == TX_SKY)
		return CM_GET(192, 255, 255);
	if (side->flags & PASSABLE)
		return CM_GET(192, 192, 192);
	return CM_GET(255, 255, 128);
}
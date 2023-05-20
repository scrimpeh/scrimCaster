#include <render/decal.h>

#include <game/camera.h>
#include <map/map.h>
#include <render/color/colormap.h>
#include <render/color/colorramp.h>
#include <render/viewport.h>
#include <util/mathutil.h>

// Debug only
#include <render/renderutil.h>

// The general idea with decal drawing is I draw them decal by decal.
// For every floor / ceiling decal, I find the four corner points in the viewport to derive a maximum bounding box.
// Every pixel in the bounding box is then scanned to find the corresponding decal pixel and paint it on the floor / ceiling,
// minding the z-buffer.

// For walls, I can find the leftmost and rightmost columns, and do the same by casting a ray for every pixel against the plane the decal rests on

// It's worth nothing that a decal can overlap multiple surfaces (up to two for walls, up to four for walls / ceils),
// so it might be easier to just split them accordingly.

u16 r_decal_dynamic_max = 30;

// For now, decals use the same graphics as sprites
static const r_decal_static R_DECALS[] =
{
	[0] = { .sheet = 0, .x = 0, .y = 0,  .w = 0,  .h = 0 }, // Dummy
	[1] = { .sheet = 0, .x = 0, .y = 0, .w = 28, .h = 64 }
};

r_decal_world* r_decals_map = NULL;
r_decal_world* r_decals_dynamic = NULL;

static u16* r_decal_dynamic_slots = NULL;
static u16 r_decal_dynamic_next_slot = 0;

static void r_decal_clear(r_decal_world* decal)
{
	decal->type = 0;
	decal->id = (m_side_id) { .orientation = M_NORTH, .x = 0, .y = 0 };
	decal->ttl = 0;
	decal->x = 0;
	decal->y = 0;
}

i32 r_decal_load()
{
	r_decal_unload();

	r_decals_map = SDL_malloc(sizeof(r_decal_world) * m_map.decal_count);
	if (!r_decals_map)
	{
		r_decal_unload();
		return -1;
	}
	for (u32 i = 0; i < m_map.decal_count; i++)
		SDL_memcpy(&r_decals_map[i], &m_map.decals[i], sizeof(r_decal_world));

	r_decals_dynamic = SDL_malloc(sizeof(r_decal_world) * r_decal_dynamic_max);
	if (!r_decals_dynamic)
	{
		r_decal_unload();
		return -1;
	}
	for (u32 i = 0; i < r_decal_dynamic_max; i++)
		r_decal_clear(&r_decals_dynamic[i]);

	r_decal_dynamic_slots = SDL_malloc(sizeof(u16) * r_decal_dynamic_max);
	if (!r_decal_dynamic_slots)
	{
		r_decal_unload();
		return -1;
	}

	r_decal_dynamic_next_slot = 0;
	for (u32 i = 0; i < r_decal_dynamic_max; i++)
		r_decal_dynamic_slots[i] = i;
	return 0;
}

void r_decal_add_dynamic(const r_decal_world* decal)
{
	const u16 next_slot = r_decal_dynamic_slots[r_decal_dynamic_next_slot++];
	r_decal_dynamic_next_slot %= r_decal_dynamic_max;
	SDL_memcpy(&r_decals_dynamic[next_slot], decal, sizeof(r_decal_world));
}

void r_decal_unload()
{
	SDL_free(r_decals_map);
	r_decals_map = NULL;

	SDL_free(r_decals_dynamic);
	r_decals_dynamic = NULL;

	SDL_free(r_decal_dynamic_slots);
	r_decal_dynamic_slots = NULL;
}

void r_decal_draw(SDL_Surface* target)
{
	// TODO: Visibility filtering and geometric datastructure
	for (u32 i = 0; i < m_map.decal_count; i++)
		r_decal_draw_decal(target, &r_decals_map[i]);

	for (u32 i = 0; i < r_decal_dynamic_max; i++)
	{
		r_decal_world* decal = &r_decals_dynamic[i];
		if (decal->type)
		{
			r_decal_draw_decal(target, decal);
			if (decal->ttl && (--decal->ttl == 0))
			{
				r_decal_clear(decal);
				r_decal_dynamic_slots[r_decal_dynamic_next_slot] = i;
			}
		}
	}
}

static void r_decal_draw_decal(SDL_Surface* target, const r_decal_world* decal)
{
	if (!r_decal_visible(decal))
		return;

	float decal_x;
	float decal_y;
	float decal_z;

	switch (decal->id.orientation)
	{
	case M_EAST:
		decal_x = (decal->id.x + 1) * M_CELLSIZE;
		decal_y = decal->id.y * M_CELLSIZE + decal->x;
		decal_z = decal->y;
		break;
	case M_NORTH:
		decal_x = decal->id.x * M_CELLSIZE + decal->x;
		decal_y = decal->id.y * M_CELLSIZE;
		decal_z = decal->y;
		break;
	case M_WEST:
		decal_x = decal->id.x * M_CELLSIZE;
		decal_y = decal->id.y * M_CELLSIZE + (M_CELLSIZE - decal->x);
		decal_z = decal->y;
		break;
	case M_SOUTH:
		decal_x = decal->id.x * M_CELLSIZE + (M_CELLSIZE - decal->x);
		decal_y = (decal->id.y + 1) * M_CELLSIZE;
		decal_z = decal->y;
		break;
	case M_FLOOR:
		decal_x = decal->id.x * M_CELLSIZE + decal->x;
		decal_y = decal->id.y * M_CELLSIZE + decal->y;
		decal_z = M_CELLHEIGHT;
		break;
	case M_CEIL:
		decal_x = decal->id.x * M_CELLSIZE + decal->x;
		decal_y = decal->id.y * M_CELLSIZE + decal->y;
		decal_z = 0;
		break;
	}

	const float dx = decal_x - viewport_x;
	const float dy = decal_y - viewport_y;
	const angle_f slope = angle_get_deg_f(dx, -dy);
	const angle_rad_f angle = TO_RADF(viewport_angle - slope);

	const float distance = math_dist_f(viewport_x, viewport_y, decal_x, decal_y);
	const float distance_corrected = distance * cosf(angle);

	const angle_f fov_min_half = angle_normalize_deg_f(viewport_angle + viewport_x_fov / 2);
	const angle_f fov_max_half = angle_normalize_deg_f(viewport_angle - viewport_x_fov / 2);
	const angle_f viewport_angle = angle_normalize_deg_f(fov_min_half - slope - (viewport_x_fov / 2));

	const i32 x = viewport_angle_to_x(TO_RADF(viewport_angle));
	const i32 y = viewport_distance_to_y(distance_corrected, decal_z);

	r_draw_pixel(target, x, y, CM_GET(255, 255, 255));
	r_draw_pixel(target, x + 1, y, CM_GET(255, 0, 255));
	r_draw_pixel(target, x, y + 1, CM_GET(255, 0, 255));
	r_draw_pixel(target, x - 1, y, CM_GET(255, 0, 255));
	r_draw_pixel(target, x, y - 1, CM_GET(255, 0, 255));
}

static bool r_decal_visible(const r_decal_world* decal)
{
	float x_a;
	float x_b;
	float y_a;
	float y_b;

	const r_decal_static* static_decal = &R_DECALS[decal->type];

	// Cull backfaces
	const angle_f viewport_angle_min = angle_normalize_deg_f(viewport_angle + viewport_x_fov / 2);
	const angle_f viewport_angle_max = angle_normalize_deg_f(viewport_angle - viewport_x_fov / 2);
	switch (decal->id.orientation)
	{
	case M_EAST:
		if (viewport_angle_max > 90 && viewport_angle_min < 270)
			return false;
		x_a = (decal->id.x + 1) * M_CELLSIZE;
		y_a = decal->id.y * M_CELLSIZE + decal->x - (static_decal->w / 2);
		x_b = x_a;
		y_b = decal->id.y * M_CELLSIZE + decal->x + (static_decal->w / 2);
		break;
	case M_NORTH:
		if (viewport_angle_max > 180 && viewport_angle_min > viewport_angle_max)
			return false;
		x_a = decal->id.x * M_CELLSIZE + decal->x - (static_decal->w / 2);
		y_a = decal->id.y * M_CELLSIZE;
		x_b = decal->id.x * M_CELLSIZE + decal->x + (static_decal->w / 2);
		y_b = y_a;
		break;
	case M_WEST:
		if (viewport_angle_max > 270 && viewport_angle_min < 90)
			return false;
		x_a = decal->id.x * M_CELLSIZE;
		y_a = decal->id.y * M_CELLSIZE + (M_CELLSIZE - decal->x) + (static_decal->w / 2);
		x_b = x_a;
		y_b = decal->id.y * M_CELLSIZE + (M_CELLSIZE - decal->x) - (static_decal->w / 2);
		break;
	case M_SOUTH:
		if (viewport_angle_min < 180 && viewport_angle_max < viewport_angle_min)
			return false;
		x_a = decal->id.x * M_CELLSIZE + (M_CELLSIZE - decal->x) + (static_decal->w / 2);
		y_a = (decal->id.y + 1) * M_CELLSIZE;
		x_b = decal->id.x * M_CELLSIZE + (M_CELLSIZE - decal->x) - (static_decal->w / 2);
		y_b = y_a;
		break;
	case M_FLOOR:
	case M_CEIL:
		x_a = decal->id.x * M_CELLSIZE + decal->x - (static_decal->w / 2);
		y_a = decal->id.y * M_CELLSIZE + decal->y - (static_decal->h / 2);
		x_b = decal->id.x * M_CELLSIZE + decal->x + (static_decal->w / 2);
		y_b = decal->id.y * M_CELLSIZE + decal->y + (static_decal->h / 2);
		break;
	}
	
	return viewport_point_on_screen(x_a, y_a) ||
		   viewport_point_on_screen(x_a, y_b) ||
		   viewport_point_on_screen(x_b, y_a) ||
		   viewport_point_on_screen(x_b, y_b);
}

const r_decal_static* r_decal_get_static(const r_decal_world* world_decal)
{
	return &R_DECALS[world_decal->type];
}
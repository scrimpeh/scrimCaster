#include <render/decal.h>

#include <game/camera.h>
#include <map/map.h>
#include <render/color/colormap.h>
#include <render/color/colorramp.h>
#include <render/gfxloader.h>
#include <render/viewport.h>
#include <util/mathutil.h>


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
	[0] = { .sheet = 0,  .x = 0,  .y = 0,  .w = 0,  .h = 0 }, // Dummy
	[1] = { .sheet = 0, .x = 28,  .y = 0, .w = 26, .h = 27 }, // Window
	[2] = { .sheet = 0, .x = 56,  .y = 0, .w = 11, .h = 12 }, // Bullet Hole 1
	[3] = { .sheet = 0, .x = 56, .y = 12, .w = 11, .h = 13 }, // Bullet Hole 2
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

void r_decal_update(u32 t_delta)
{
	for (u32 i = 0; i < r_decal_dynamic_max; i++)
	{
		r_decal_world* decal = &r_decals_dynamic[i];
		if (decal->type)
		{
			if (decal->ttl)
			{
				decal->ttl -= t_delta;
				if (decal->ttl < 0)
				{
					r_decal_clear(decal);
					r_decal_dynamic_slots[r_decal_dynamic_next_slot] = i;
				}
			}
		}
	}
}

const r_decal_static* r_decal_get_static(const r_decal_world* world_decal)
{
	return &R_DECALS[world_decal->type];
}

void r_decal_get_map_bounds(const r_decal_world* decal, r_decal_bounds* bounds)
{
	SDL_assert(decal->type);

	// TODO: Account for things like mirroring or rotation here

	const r_decal_static* static_decal = &R_DECALS[decal->type];

	switch (decal->id.orientation)
	{
	case M_FLOOR:
	case M_CEIL:
	{
		const i32 wx = decal->id.x * M_CELLSIZE + decal->x;
		const i32 wy = decal->id.y * M_CELLSIZE + decal->y;
		bounds->x_a = (wx - (static_decal->w / 2));
		bounds->y_a = (wy - (static_decal->h / 2));
		bounds->x_b = (wx + (static_decal->w / 2));
		bounds->y_b = (wy + (static_decal->h / 2));
		return;
	}
	case M_NORTH:
	case M_SOUTH:
	{
		const i32 wx_offs = decal->id.orientation == M_NORTH ? decal->x : M_CELLSIZE - 1 - decal->x;
		const i32 wx = decal->id.x * M_CELLSIZE + wx_offs;
		bounds->x_a = (wx - (static_decal->w / 2));
		bounds->y_a = decal->id.y * M_CELLSIZE;
		bounds->x_b = (wx + (static_decal->w / 2));
		bounds->y_b = decal->id.y * M_CELLSIZE;
		return;
	}
	case M_EAST:
	case M_WEST:
	{
		const i32 wy_offs = decal->id.orientation == M_EAST ? decal->x : M_CELLSIZE - 1 - decal->x;
		const i32 wy = decal->id.y * M_CELLSIZE + wy_offs;
		bounds->x_a = decal->id.x * M_CELLSIZE;
		bounds->y_a = (wy - (static_decal->w / 2));
		bounds->x_b = decal->id.x * M_CELLSIZE;
		bounds->y_b = (wy - (static_decal->w / 2));
		return;
	}
	}
}

i16 r_decal_get_col(const r_decal_world* decal, i16 mx, i16 my, u8 side_col)
{
	SDL_assert(decal->type);
	SDL_assert(decal->id.orientation != M_FLOOR && decal->id.orientation != M_CEIL);

	// TODO: Account for things like mirroring or rotation here

	const r_decal_static* static_decal = &R_DECALS[decal->type];

	i16 col_decal;
	i16 col_current;
	switch (decal->id.orientation)
	{
	case M_EAST:  
		col_decal = decal->id.y * M_CELLSIZE + decal->x;           
		col_current = my * M_CELLSIZE + side_col;
		break;
	case M_NORTH: 
		col_decal = decal->id.x * M_CELLSIZE + decal->x;
		col_current = mx * M_CELLSIZE + side_col;
		break;
	case M_WEST:  
		col_decal = decal->id.y * M_CELLSIZE + M_CELLSIZE - 1 - decal->x;
		col_current = my * M_CELLSIZE + M_CELLSIZE - 1 - side_col;
		break;
	case M_SOUTH: 
		col_decal = decal->id.x * M_CELLSIZE + M_CELLSIZE - 1 - decal->x;
		col_current = mx * M_CELLSIZE + M_CELLSIZE - 1 - side_col;
		break;
	}
	
	return col_current - (col_decal - (static_decal->w / 2));
}

r_decal_pt r_decal_get_pt(const r_decal_world* decal, i32 wx, i32 wy)
{
	SDL_assert(decal->type);
	SDL_assert(decal->id.orientation == M_FLOOR || decal->id.orientation == M_CEIL);

	// TODO: Account for things like mirroring or rotation here

	const r_decal_static* static_decal = &R_DECALS[decal->type];

	const i32 wx_decal = decal->id.x * M_CELLSIZE + decal->x;
	const i32 wy_decal = decal->id.y * M_CELLSIZE + decal->y;

	const i32 wx_origin = wx_decal - static_decal->w / 2;
	const i32 wy_origin = wy_decal - static_decal->h / 2;

	r_decal_pt pt;
	pt.x = wx - wx_origin;
	pt.y = wy - wy_origin;
	return pt;
}

bool r_decal_in_bounds(const r_decal_static* decal, r_decal_pt pt)
{
	return pt.x >= 0 && pt.x <= decal->w && pt.y >= 0 && pt.y <= decal->h;
}

const cm_color r_decal_get_px(const r_decal_static* decal, r_decal_pt pt)
{
	const u32* pixels = gfx_ws_buffer[decal->sheet]->pixels;
	return pixels[(decal->y + pt.y) * gfx_ws_buffer[decal->sheet]->w + (decal->x + pt.x)];
}
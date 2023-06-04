#include <render/scan.h>

#include <game/camera.h>
#include <geometry.h>
#include <map/block/blockmap.h>
#include <map/block/block_iterator.h>
#include <map/map.h>
#include <render/color/colormap.h>
#include <render/color/colorramp.h>
#include <render/lighting/lighting.h>
#include <render/render.h>
#include <render/skybox.h>
#include <render/texture.h>
#include <render/viewport.h>
#include <util/mathutil.h>

#include <math.h>
#include <float.h>

block_iterator* scan_sprite_iter = NULL;

// For drawing transparent surfaces, we keep a stack of (dynamically allocated) draw side
scan_intercept_stack* scan_intercepts = NULL;

static bool scan_collect_intercept(const g_intercept* intercept)
{
	scan_intercept_stack* store_intercept = SDL_malloc(sizeof(scan_intercept_stack));
	if (!store_intercept)
		return false;
	SDL_memcpy(&store_intercept->intercept, intercept, sizeof(g_intercept));
	store_intercept->next = scan_intercepts;
	scan_intercepts = store_intercept;
	return intercept->type == G_INTERCEPT_NON_SOLID;
}

static bool scan_collect_cell(i16 mx, i16 my)
{
	block_iterator_add_cell(scan_sprite_iter, block_get_pt(mx, my));
	return true;
}

void scan_draw(SDL_Surface* target)
{
	for (u16 col = 0; col < viewport_w; col++)
	{
		const angle_rad_f angle = viewport_x_to_angle(TO_RADF(viewport_angle), col);
		g_cast(viewport_x, viewport_y, angle, scan_collect_intercept, scan_collect_cell);

		while (scan_intercepts)
		{
			scan_intercept_stack* cur_intercept = scan_intercepts;
			scan_draw_column(target, viewport_x, viewport_y, &cur_intercept->intercept, col);
			scan_intercepts = scan_intercepts->next;
			SDL_free(cur_intercept);
		}
	}
}

static u8 scan_get_slice_y_start(const m_side* side)
{
	if (!side->flags & DOOR_V)
		return 0;
	return side->door.scroll;
}

static u8 scan_get_tx_slice_y(i64 wall_h, i64 y, u8 start_y)
{
	const i64 y_top = (viewport_h - wall_h) / 2;
	const i64 y_rel = y - y_top;
	const float tx = (float) y_rel / wall_h * TX_SIZE;
	return (u8) tx + start_y;
}

static void scan_draw_column(SDL_Surface* target, float x, float y, const g_intercept* intercept, u16 col)
{
	// For doors, we basically have to work out how open the door is (presumably as a ratio between
	// total height and pixel height, and what pixel to start dawing at
	// -> this is gonna become spaghetti code super fast.

	// Do triangular correction on the distance
	const angle_rad_f angle = TO_RADF(viewport_angle) - intercept->angle;
	const float distance = math_dist_f(x, y, intercept->wx, intercept->wy);
	const float distance_corrected = distance * cosf(angle);

	// Round up the wall height to the nearest multiple of two so there's an equal number of pixels
	// below and above the wall. This simplifies floor rendering at the cost of some accuracy.
	i32 wall_h = viewport_distance_to_length(R_CELL_H, distance_corrected);
	if (wall_h & 1)
		wall_h++;
	i32 wall_y = (viewport_h - wall_h) / 2;

	// Get the texture
	const m_side* side = m_get_side(intercept->mx, intercept->my, intercept->orientation);
	const tx_slice slice = tx_get_slice(side, intercept->column);

	// Now draw the texture slice
	i32 y_top = SDL_max(0, wall_y);
	i32 y_end = viewport_h - y_top;

	if (side->type == TX_SKY)
	{
		r_sky_draw(target, col, y_top, y_end, intercept->angle);
		for (u32 y = y_top; y < y_end; y++)
			viewport_z_buffer[viewport_w * y + col] = FLT_MAX;
	}
	else
	{
		if (side->flags & DOOR_V)
		{
			const i32 door_h = (i32) (wall_h * ((R_CELL_H - side->door.scroll) / (float) R_CELL_H));
			y_end = SDL_min(viewport_h, wall_y + door_h);
		}

		u32* render_px = (u32*) target->pixels + (y_top * viewport_w) + col;
		float* z_buffer_px = viewport_z_buffer + (y_top * viewport_w) + col;

		// Calculate lighting and distance fog for side
		const float brightness = r_light_get_alpha(intercept->mx, intercept->my, intercept->orientation, intercept->column, 0);
		const cm_alpha_color distance_fog = cm_ramp_get_px(distance_corrected);
		
		const u8 slice_start = scan_get_slice_y_start(side);
		// First, draw pure wall slice
		for (i32 draw_y = y_top; draw_y < y_end; draw_y++)
		{
			u32 tex_col = *(slice + scan_get_tx_slice_y(wall_h, draw_y, slice_start));
			if (tex_col != COLOR_KEY)
			{
				*render_px = tex_col;
				*z_buffer_px = distance_corrected;
			}

			render_px += viewport_w;
			z_buffer_px += viewport_w;
		}

		// Now check what slices the decal contains
		const block_ref_list_entry* decal_ref = block_ref_list_get(block_get_pt(intercept->mx, intercept->my), BLOCK_TYPE_DECAL_SIDE)->first;
		while (decal_ref)
		{
			const r_decal_world* decal = decal_ref->reference;
			if (decal->id.orientation == intercept->orientation)
			{
				const r_decal_static* static_decal = r_decal_get_static(decal);
				const i16 decal_col = r_decal_get_col(decal, intercept->mx, intercept->my, intercept->column);
				if (decal_col >= 0 && decal_col <= static_decal->w)
				{
					// There is a decal column to draw, find the start and end height of the decal.
					// Align the decal to the texture grid - to do this, we invert the texture pixel mapping  (scan_get_tx_slice_y)

					const i32 decal_wz_top = decal->y - static_decal->h / 2 - slice_start;
					const i32 decal_sz_top = ceil(((float) decal_wz_top / TX_SIZE) * wall_h) + wall_y;

					const i32 decal_wz_end = decal_wz_top + static_decal->h + 1;
					const i32 decal_sz_end = ceil(((float) decal_wz_end / TX_SIZE) * wall_h) + wall_y;

					const i32 decal_draw_start = SDL_max(decal_sz_top, y_top);
					const i32 decal_draw_end = SDL_min(decal_sz_end, y_end);

					render_px = (u32*) target->pixels + (decal_draw_start * viewport_w) + col;

					for (i32 draw_y = decal_draw_start; draw_y < decal_draw_end; draw_y++)
					{
						const u8 slice_px = scan_get_tx_slice_y(wall_h, draw_y, slice_start);
						r_decal_pt pt;
						pt.x = decal_col;
						pt.y = slice_px - decal->y + (static_decal->h / 2);
						const cm_color decal_px = r_decal_get_px(static_decal, pt);
						if (decal_px != COLOR_KEY && *(slice + slice_px) != COLOR_KEY)
							*render_px = decal_px;

						render_px += viewport_w;
					}
				}
			}

			decal_ref = decal_ref->next;
		}

		// Finally, apply distance fog and lighting
		render_px = (u32*) target->pixels + (y_top * viewport_w) + col;
		for (i32 draw_y = y_top; draw_y < y_end; draw_y++)
		{
			const u32 tex_col = *(slice + scan_get_tx_slice_y(wall_h, draw_y, slice_start));
			if (tex_col != COLOR_KEY)
				*render_px = cm_ramp_apply(r_light_apply(*render_px, brightness), distance_fog);
			render_px += viewport_w;
		}
	}

	// Now draw the ceiling and floor
	// For this, we invert the projection and cosine correction to get the wall height

	// No need to draw another floor, because we've already done so from the previous wall
	if (intercept->type == G_INTERCEPT_NON_SOLID)
		return;

	u32* render_px_ceil = (u32*) target->pixels + ((y_top - 1) * viewport_w) + col;
	u32* render_px_floor = (u32*) target->pixels + ((viewport_h - y_top) * viewport_w) + col;
	float* z_buffer_px_ceil = viewport_z_buffer + ((y_top - 1) * viewport_w) + col;
	float* z_buffer_px_floor = viewport_z_buffer + ((viewport_h - y_top) * viewport_w) + col;

	for (i32 y_px = y_top - 1; y_px != -1; y_px--)
	{
		const float d = viewport_y_to_distance(y_px);
		math_vec_2f w_target = math_vec_cast_f(x, y, intercept->angle, d / cosf(angle));

		const i16 mx = w_target.x / M_CELLSIZE;
		const i16 my = w_target.y / M_CELLSIZE;

		const i16 cx = fmodf(w_target.x, M_CELLSIZE);
		const i16 cy = fmodf(w_target.y, M_CELLSIZE);

		const float brightness = r_light_get_alpha(mx, my, M_FLOOR, cx, cy);
		const cm_alpha_color distance_fog = cm_ramp_get_px(d);

		const u32 floor_px = scan_get_flat_px(mx, my, cx, cy, M_FLOOR);
		if (floor_px == COLOR_KEY)
		{
			*render_px_floor = r_sky_get_pixel(viewport_h - y_px - 1, intercept->angle);
			*z_buffer_px_floor = FLT_MAX;
		}
		else
		{
			*render_px_floor = cm_ramp_apply(r_light_apply(floor_px, brightness), distance_fog);
			*z_buffer_px_floor = d;
		}

		const u32 ceil_px = scan_get_flat_px(mx, my, cx, cy, M_CEIL);
		if (ceil_px == COLOR_KEY)
		{
			*render_px_ceil = r_sky_get_pixel(y_px, intercept->angle);
			*z_buffer_px_ceil = FLT_MAX;
		}
		else
		{
			*render_px_ceil = cm_ramp_apply(r_light_apply(ceil_px, brightness), distance_fog);
			*z_buffer_px_ceil = d;
		}

		render_px_ceil -= viewport_w;
		render_px_floor += viewport_w;
		z_buffer_px_ceil -= viewport_w;
		z_buffer_px_floor += viewport_w;
	}
}

static cm_color scan_get_flat_px(i16 mx, i16 my, u8 cx, u8 cy, m_orientation orientation)
{
	const m_cell* cell = m_get_cell(mx, my);
	const cm_color px = tx_get_point(orientation == M_CEIL ? &cell->ceil : &cell->floor, cx, cy);
	if (px == COLOR_KEY)
		return px;

	// Look for a decal pixel
	// TODO: (Optional) Alpha blending
	const block_ref_list_entry* decal_ref = block_ref_list_get(block_get_pt(mx, my), BLOCK_TYPE_DECAL_FLAT)->first;
	while (decal_ref)
	{
		const r_decal_world* decal = decal_ref->reference;
		if (decal->id.orientation == orientation)
		{
			const r_decal_static* static_decal = r_decal_get_static(decal);
			const r_decal_pt decal_pt = r_decal_get_pt(decal, mx * M_CELLSIZE + cx, my * M_CELLSIZE + cy);
			if (r_decal_in_bounds(static_decal, decal_pt))
			{
				const cm_color decal_px = r_decal_get_px(static_decal, decal_pt);
				if (decal_px != COLOR_KEY)
					return decal_px;
			}
		}

		decal_ref = decal_ref->next;
	}

	return px;
}
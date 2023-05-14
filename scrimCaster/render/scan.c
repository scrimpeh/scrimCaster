#include <render/scan.h>

#include <game/camera.h>
#include <geometry.h>
#include <map/map.h>
#include <render/color/colormap.h>
#include <render/color/colorramp.h>
#include <render/lighting/lighting.h>
#include <render/render.h>
#include <render/renderconstants.h>
#include <render/skybox.h>
#include <render/texture.h>
#include <render/viewport.h>
#include <util/mathutil.h>

#include <math.h>	//fmod is obsolete: maybe replace?
#include <float.h>

// For drawing transparent surfaces, we keep a stack of (dynamically allocated) draw side
g_intercept_stack* intercept_stack = NULL;


static inline bool collect_intercept(const g_intercept* intercept)
{
	g_intercept_stack* store_intercept = SDL_malloc(sizeof(g_intercept_stack));
	if (!store_intercept)
		return false;
	SDL_memcpy(&store_intercept->intercept, intercept, sizeof(g_intercept));
	store_intercept->next = intercept_stack;
	intercept_stack = store_intercept;
	return intercept->type == G_INTERCEPT_NON_SOLID;
}

void scan_draw(SDL_Surface* target)
{
	for (u16 col = 0; col < viewport_w; col++)
	{
		const angle_rad_f angle = viewport_x_to_angle(TO_RADF(viewport_angle), col);
		g_cast(viewport_x, viewport_y, angle, collect_intercept);

		while (intercept_stack) 
		{
			g_intercept_stack* cur_intercept = intercept_stack;
			scan_draw_column(target, viewport_x, viewport_y, &cur_intercept->intercept, col);
			intercept_stack = intercept_stack->next;
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
	const i64 wall_y = (viewport_h - wall_h) / 2;
	const i64 y_rel = y - wall_y;
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
	const float distance = math_dist_f(x, y, intercept->x, intercept->y);
	const float distance_corrected = distance * cosf(angle);

	// Round up the wall height to the nearest multiple of two so there's an equal number of pixels
	// below and above the wall. This simplifies floor rendering at the cost of some accuracy.
	i32 wall_h = (i32) (viewport_projection_distance * R_CELL_H / distance_corrected);
	if (wall_h & 1)
		wall_h++;
	i32 wall_y = (viewport_h - wall_h) / 2;

	// Get the texture
	const m_side* side = m_get_side(intercept->map_x, intercept->map_y, intercept->orientation);
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
			const i32 door_h = (i32) (wall_h * ((R_CELL_H - side->door.scroll) / (float) (R_CELL_H)));
			y_end = SDL_min(viewport_h, wall_y + door_h);
		}

		u32* render_px = (u32*) target->pixels;
		float* z_buffer_px = viewport_z_buffer;

		render_px += (y_top * viewport_w) + col;
		z_buffer_px += (y_top * viewport_w) + col;

		for (i32 draw_y = y_top; draw_y < y_end; ++draw_y)
		{
			const u8 slice_px = scan_get_tx_slice_y(wall_h, draw_y, scan_get_slice_y_start(side));
			u32 tex_col = *(slice + slice_px);
			if (tex_col != COLOR_KEY)
			{
				tex_col = r_light_px(intercept->map_x, intercept->map_y, intercept->orientation, tex_col, intercept->column, slice_px);
				tex_col = cm_ramp_mix(tex_col, distance_corrected);

				*render_px = tex_col;
				*z_buffer_px = distance_corrected;
			}

			render_px += viewport_w;
			z_buffer_px += viewport_w;
		}
	}

	// Now draw the ceiling and floor
	// For this, we invert the projection and cosine correction to get the wall height

	// No need to draw another floor, because we've already done so from the previous wall
	if (intercept->type == G_INTERCEPT_NON_SOLID)
		return;

	u32* floor_render_px_top = (u32*) target->pixels + ((y_top - 1) * viewport_w) + col;
	u32* floor_render_px_bottom = (u32*) target->pixels + ((viewport_h - y_top) * viewport_w) + col;
	float* z_buffer_px_top = viewport_z_buffer + ((y_top - 1) * viewport_w) + col;
	float* z_buffer_px_bottom = viewport_z_buffer + ((viewport_h - y_top) * viewport_w) + col;

	for (i32 y_px = y_top - 1; y_px != -1; y_px--)
	{
		const float d = viewport_y_to_distance(y_px);
		float xa;
		float ya;
		math_vec_cast_f(x, y, intercept->angle, d / cosf(angle), &xa, &ya);

		const i16 grid_xa = (i16) (floorf(xa / M_CELLSIZE));
		const i16 grid_ya = (i16) (floorf(ya / M_CELLSIZE));

		const i16 cell_x = (i16) (fmodf(floorf(xa), (float)(M_CELLSIZE)));
		const i16 cell_y = (i16) (fmodf(floorf(ya), (float)(M_CELLSIZE)));

		const m_cell* cell = m_get_cell(grid_xa, grid_ya);

		u32 floor_px = tx_get_point(&cell->floor, cell_x, cell_y);
		if (floor_px == COLOR_KEY)
		{
			*floor_render_px_bottom = r_sky_get_pixel(viewport_h - y_px - 1, intercept->angle);
			*z_buffer_px_bottom = FLT_MAX;
		}
		else
		{
			floor_px = r_light_px(grid_xa, grid_ya, M_FLOOR, floor_px, cell_x, cell_y);
			floor_px = cm_ramp_mix(floor_px, d);
			*floor_render_px_bottom = floor_px;
			*z_buffer_px_bottom = d;
		}

		u32 ceil_px = tx_get_point(&cell->ceil, cell_x, cell_y);
		if (ceil_px == COLOR_KEY)
		{
			*floor_render_px_top = r_sky_get_pixel(y_px, intercept->angle);
			*z_buffer_px_top = FLT_MAX;
		}
		else
		{
			ceil_px = r_light_px(grid_xa, grid_ya, M_CEIL, ceil_px, cell_x, cell_y);
			ceil_px = cm_ramp_mix(ceil_px, d);
			*floor_render_px_top = ceil_px;
			*z_buffer_px_top = d;
		}

		floor_render_px_top -= viewport_w;
		floor_render_px_bottom += viewport_w;
		z_buffer_px_top -= viewport_w;
		z_buffer_px_bottom += viewport_w;
	}
}


#include <render/scan.h>

#include <camera.h>
#include <geometry.h>
#include <map/map.h>
#include <map/maputil.h>
#include <render/colormap.h>
#include <render/colorramp.h>
#include <render/lighting.h>
#include <render/render.h>
#include <render/renderconstants.h>
#include <render/skybox.h>
#include <render/texture.h>
#include <util/mathutil.h>

#include <math.h>	//fmod is obsolete: maybe replace?
#include <float.h>

extern Map m_map;
extern u8 viewport_x_fov;
u8 viewport_x_fov_half;

u16 viewport_w_half;

float projection_dist;

// The Z Buffer is used for masking sprites against the wall. It covers every pixel on the screen to support blending spriets
// against transparency layers
float* z_buffer;

// For drawing transparent surfaces, we keep a stack of (dynamically allocated) draw side
g_intercept_stack* intercept_stack = NULL;

float* angle_offsets;

i32 scan_init()
{
	// Set up global rendering parameters
	viewport_w_half = viewport_w / 2;
	viewport_x_fov_half = viewport_x_fov / 2;
	projection_dist = viewport_w_half / tanf(TO_RADF(viewport_x_fov_half));

	angle_offsets = SDL_malloc(sizeof(float) * viewport_w);
	if (!angle_offsets)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR,"Couldn't initialize offset buffer! %s", SDL_GetError());
		return -1;
	}

	for (u16 i = 0; i < viewport_w; i++)
		angle_offsets[i] = atanf((i - viewport_w_half) / projection_dist);

	z_buffer = SDL_malloc(sizeof(float) * viewport_w * viewport_h);
	if (!z_buffer) 
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't initialize Z Buffer! %s", SDL_GetError());
		return -1;
	}

	return 0;
}

// To be called whenever the viewport FOV is changed
void scan_close()
{
	SDL_free(z_buffer);
	z_buffer = NULL;

	SDL_free(angle_offsets);
	angle_offsets = NULL;
}

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
	// Clear Z Buffer
	for (u64 i = 0; i < (u64) viewport_w * viewport_h; i++)
		z_buffer[i] = FLT_MAX;

	const float angle_rad = TO_RADF(viewport_angle);
	for (u16 col = 0; col < viewport_w; col++)
	{
		const float angle = angle_normalize_rad_f(angle_rad - angle_offsets[col]);
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

static u8 scan_get_slice_y_start(const Side* side)
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

static i32 scan_get_wall_height(i16 height, float distance, angle_rad_f angle)
{
	// Round up the wall height to the nearest multiple of two so there's an equal number of pixels
	// below and above the wall. This simplifies floor rendering at the cost of some accuracy.
	distance *= cosf(angle);
	i32 h = (i32) projection_dist * height / distance;
	return h + (h & 1);
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
	i32 wall_h = (i32) (projection_dist * R_CELL_H / distance_corrected);
	if (wall_h & 1)
		wall_h++;
	i32 wall_y = (viewport_h - wall_h) / 2;

	// Get the texture
	const Side* side = map_get_side(intercept->map_x, intercept->map_y, intercept->orientation);
	const tx_slice slice = tx_get_slice(side, intercept->column);

	// Now draw the texture slice
	i32 y_top = SDL_max(0, wall_y);
	i32 y_end = viewport_h - y_top;

	if (side->type == TX_SKY)
		r_sky_draw(target, col, y_top, y_end, intercept->angle);
	else
	{
		if (side->flags & DOOR_V)
		{
			const i32 door_h = (i32) (wall_h * ((R_CELL_H - side->door.scroll) / (float) (R_CELL_H)));
			y_end = SDL_min(viewport_h, wall_y + door_h);
		}

		u32* render_px = (u32*) target->pixels;
		float* z_buffer_px = z_buffer;

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

	for (i32 y_px = y_top - 1; y_px != -1; y_px--)
	{
		const u16 height = viewport_h - (2 * y_px);
		const float d = (projection_dist * R_CELL_H) / height;
		float xa;
		float ya;
		math_vec_cast_f(x, y, intercept->angle, d / cosf(angle), &xa, &ya);

		const i16 grid_xa = (i16) (floorf(xa / M_CELLSIZE));
		const i16 grid_ya = (i16) (floorf(ya / M_CELLSIZE));

		const i16 cell_x = (i16) (fmodf(floorf(xa), (float)(M_CELLSIZE)));
		const i16 cell_y = (i16) (fmodf(floorf(ya), (float)(M_CELLSIZE)));

		const m_flat* flat = &m_get_cell(grid_xa, grid_ya)->flat;

		u32 floor_px = tx_get_point(flat, cell_x, cell_y, true);
		if (floor_px == COLOR_KEY)
			*floor_render_px_bottom = r_sky_get_pixel(viewport_h - y_px - 1, intercept->angle);
		else
		{
			floor_px = r_light_px(grid_xa, grid_ya, M_FLOOR, floor_px, cell_x, cell_y);
			floor_px = cm_ramp_mix(floor_px, d);
			*floor_render_px_bottom = floor_px;
		}

		u32 ceil_px = tx_get_point(flat, cell_x, cell_y, false);
		if (ceil_px == COLOR_KEY)
			*floor_render_px_top = r_sky_get_pixel(y_px, intercept->angle);
		else
		{
			ceil_px = r_light_px(grid_xa, grid_ya, M_CEIL, ceil_px, cell_x, cell_y);
			ceil_px = cm_ramp_mix(ceil_px, d);
			*floor_render_px_top = ceil_px;
		}

		floor_render_px_top -= viewport_w;
		floor_render_px_bottom += viewport_w;
	}
}


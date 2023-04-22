#include "scan.h"

#include "map.h"
#include "maputil.h"
#include "camera.h"
#include "geometry.h"
#include "render.h"
#include "renderconstants.h"

#include "SDL/SDL_assert.h"
#include "SDL/SDL_log.h"
#include <math.h>	//fmod is obsolete: maybe replace?
#include <float.h>

// How many columsn to draw before we stop looking
#define MAXDIST 32

const float WALL_OFF = 8e-4f;
#define HALFSIZE (TEX_SIZE / 2)

extern u64 ticks;

extern float viewport_x;
extern float viewport_y;
extern float viewport_angle;

extern Map map;
extern u8 viewport_x_fov;
u8 viewport_x_fov_half;

// For drawing textures
extern u8 fillCount;
extern u8 MAX_TEXTURE_BUF;
extern SDL_Surface* mapTextureBuffer[];

extern u16 viewport_w, viewport_h;
u16 viewport_w_half;
u8 colwidth = 1;

const float MAXSLOPE = 1e+8f;

float projection_dist;
// The Z Buffer is used for masking sprites against the wall. It covers every pixel on the screen to support blending spriets
// against transparency layers
float* z_buffer;

// For drawing transparent surfaces, we keep a stack of (dynamically allocated) draw side
g_intercept_stack* intercept_stack = NULL;

float* angle_offsets;

// Set up the scanning parameters as needed
i32 scan_init(u8 column_width)
{
	// Set up global rendering parameters
	colwidth = column_width;

	// Derive constant values from it
	viewport_w_half = viewport_w / 2;
	viewport_x_fov_half = viewport_x_fov / 2;
	projection_dist = viewport_w_half / tanf(TO_RADF(viewport_x_fov_half));

	angle_offsets = (float*)SDL_malloc(sizeof(float) * viewport_w);
	if (!angle_offsets)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR,"Couldn't initialize offset buffer! %s", SDL_GetError());
		return -1;
	}

	for (u16 i = 0; i < viewport_w; ++i)
		angle_offsets[i] = atanf((i - viewport_w_half) / projection_dist);

	z_buffer = (float*)SDL_malloc(sizeof(float) * viewport_w * viewport_h);
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
	if (intercept->type == G_INTERCEPT_VOID)
		return false;
	g_intercept_stack* store_intercept = (g_intercept_stack*) SDL_malloc(sizeof(g_intercept_stack));
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
	for (u64 i = 0; i < (u64)viewport_w * viewport_h; i++)
		z_buffer[i] = FLT_MAX;

	const float angle_rad = TO_RADF(viewport_angle);
	for (u16 col = 0; col < viewport_w; col += colwidth)
	{
		float angle = angle_rad - angle_offsets[col];
		while (angle < 0)
			angle += (float)PI_2_1;
		while (angle >= PI_2_1)
			angle -= (float)PI_2_1;

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

static void scan_draw_column(SDL_Surface* target, float x, float y, const g_intercept* intercept, u16 col)
{
	// For doors, we basically have to work out how open the door is (presumably as a ratio between
	// total height and pixel height, and what pixel to start dawing at
	// -> this is gonna become spaghetti code super fast.

	// Do triangular correction on the distance
	const angle_rad_f relative_angle = TO_RADF(viewport_angle) - intercept->angle;
	float dx = intercept->x - x;
	float dy = intercept->y - y;
	float distance = sqrt(pow(dx, 2) + pow(dy, 2));
	distance *= cosf(relative_angle);

	// Get the wall parameters
	i32 wall_h = (i32)(projection_dist * CELLHEIGHT / distance);
	i32 wall_y = (viewport_h - wall_h) / 2;

	// Get the texture
	const Side* side = map_get_side(intercept->map_x, intercept->map_y, intercept->orientation);
	const u8 tx_index = side->type & 0x00FF;
	u8 tx_sheet = (side->type & 0xFF00) >> 8;
	if (tx_sheet >= fillCount) 
		tx_sheet = 0;

	const u16 tex_x = intercept->side_col + (tx_index & 0x0F) * TEX_SIZE;
	const u16 tex_y = (tx_index & 0xF0) * (TEX_SIZE >> 4);

	const u32* tex_px = (u32*)mapTextureBuffer[tx_sheet]->pixels;	
	const u32* const tx = tex_px + (tex_y * TEX_MAP_SIZE + tex_x);
 
	float tex_y_px = wall_y >= 0 ? 
		0 :
		HALFSIZE - (((float)viewport_h / wall_h) * HALFSIZE);
	const float inc_ratio = (TEX_SIZE / (wall_h + WALL_OFF));

	// Now draw the texture slice
	i32 y_top = SDL_max(0, wall_y);
	i32 y_end = viewport_h - y_top;

	if (side->flags & DOOR_V)
	{
		float offset = 0.;
		if (side->door.status & 1)
			offset = (float)side->door.timer_ticks /
			( side->door.status & 2 ? -side->door.closespeed : side->door.openspeed );

		tex_y_px += side->door.scroll + offset;

		wall_h = (i32)(projection_dist * (CELLHEIGHT - side->door.scroll) / distance);
		y_end = SDL_min(viewport_h, wall_y + wall_h);
	}

	// TODO: Colum nwidth
	u32* render_px = (u32*)target->pixels;
	float* z_buffer_px = z_buffer;

	render_px += (y_top * viewport_w) + col;
	z_buffer_px += (y_top * viewport_w) + col;

	for (i32 draw_y = y_top; draw_y < y_end; ++draw_y)
	{
		tex_px = tx + (u8)tex_y_px * TEX_MAP_SIZE;
		const u32 tex_col = *tex_px;
		if (tex_col != COLOR_KEY)
		{
			*render_px = tex_col;
			*z_buffer_px = distance;
		}

		render_px += viewport_w;
		z_buffer_px += viewport_w;
		tex_y_px += inc_ratio;
	}
}
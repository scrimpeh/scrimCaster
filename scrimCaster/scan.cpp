#include "scan.h"
#include "SDL/SDL_assert.h"
#include "SDL/SDL_log.h"
#include "math.h"	//fmod is obsolete: maybe replace?
#include "float.h"

#include "map.h"
#include "camera.h"
#include "render.h"
#include "renderconstants.h"

#define MAXDIST 32
#define TRANSPARENCY_LAYERS 16

const float WALL_OFF = 8e-4f;
#define HALFSIZE (TEX_SIZE / 2)

// How many collumns to draw before we stop looking
const u8 maxDistance = MAXDIST;
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
extern float spr_ratio;

// The Z Buffer is used for masking sprites against the wall. It covers every pixel on the screen to support blending spriets
// against transparency layers
float* z_buffer;

// For drawing transparent surfaces, we keep a stack of (dynamically allocated) draw side
DrawSide* draw_side_stack = NULL;

float* angle_offsets;

// Set up the scanning parameters as needed
i32 InitializeScan(u8 collumn_width)
{
	// Set up global rendering parameters
	collumn_width = 1;
	colwidth = collumn_width;

	// Derive constant values from it
	viewport_w_half = viewport_w / 2;
	viewport_x_fov_half = viewport_x_fov / 2;
	projection_dist = viewport_w_half / tanf(TO_RADF(viewport_x_fov_half));

	angle_offsets = (float*)SDL_malloc(sizeof(float) * viewport_w);
	if (!angle_offsets)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR,
			"Couldn't initialize offset buffer! %s",
			SDL_GetError());
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
void CloseScan()
{
	SDL_free(z_buffer);
	z_buffer = NULL;

	SDL_free(angle_offsets);
	angle_offsets = 0;

}

static inline void PushDrawSide(const Side* side, float p_x, float p_y, u8 orientation)
{
	const float d_x = p_x - viewport_x;
	const float d_y = p_y - viewport_y;
	const float dist = sqrtf(powf(d_x, 2) + powf(d_y, 2));

	const i8 offset = side->flags & (SCROLL_H | DOOR_H) && side->param1 ?
		((i64)ticks / (i16)side->param1) % TEX_SIZE :
		0;

	u8 texcol = ((u8)(orientation & 1 ? p_x : p_y) + offset) % TEX_SIZE;
	if (orientation & 2) 
		texcol = (TEX_SIZE - 1) - texcol;
	if (side->flags & MIRR_H) 
		texcol = (TEX_SIZE - 1) - texcol;

	// Not sure how amazing it is that we allocate and free drawsides so frequently, but it's probably fine
	DrawSide* ds = (DrawSide*)SDL_malloc(sizeof(DrawSide));
	ds->next = draw_side_stack;
	ds->side = side;
	ds->dist = dist;
	ds->texcol = texcol;

	draw_side_stack = ds;
}

void DrawGeometry(SDL_Surface* target)
{
	DrawSide ds;
	float angle;
	const Side* cur_side;
	u8 cell_orientation;
	u8 cell_distance;
	const Cell* const cellptr = map.cells;
	u32 offset;

	// Clear Z Buffer
	for (u64 i = 0; i < viewport_w * viewport_h; i++)
		z_buffer[i] = FLT_MAX;

	const float angle_rad = TO_RADF(viewport_angle);
	for (u16 col = 0; col < viewport_w; col += colwidth)
	{

		angle = angle_rad - angle_offsets[col];
		while (angle < 0) 
			angle += (float)PI_2_1;
		while (angle >= PI_2_1) 
			angle -= (float)PI_2_1;

		cell_distance = 0;

		const bool north = angle < M_PI; 
		const bool east = angle < PI_1_2 || angle > PI_3_2;

		i16 grid_x = (i16)floorf(viewport_x / CELLSIZE);
		i16 grid_y = (i16)floorf(viewport_y / CELLSIZE);
		// Since our grid inverts the y coordinate, invert the slope
		float slope = -1 * tanf(angle);
		float p_x = viewport_x;
		float p_y = viewport_y;
		float diff_x = (!east ? grid_x : grid_x + 1) * CELLSIZE - p_x;

		if (fabsf(slope) > MAXSLOPE)
			slope = slope > 0 ? MAXSLOPE : -MAXSLOPE;
	trace:
		p_x += diff_x;
		p_y += diff_x * slope; 
		const i16 new_grid_y = (i16)floorf(p_y / CELLSIZE);
		
		// Trace grid vertically first
		for (i16 y_cell = grid_y; y_cell != new_grid_y; y_cell += north ? -1 : 1 )
		{
			if (++cell_distance >= maxDistance)
				goto drawcol;
			if (y_cell < 0 || y_cell >= map.boundsY || grid_x < 0 || grid_x >= map.boundsX)
				goto drawcol;

			offset = (map.boundsX * y_cell) + grid_x;
			cur_side = north ? &cellptr[offset].n : &cellptr[offset].s;
			if (cur_side->type)
			{
				// We hit something - invert the sloping operation to
				// determine the exact position of impact
				const float p_x_old = p_x;
				const float p_y_old = p_y;
				const i16 y_cell_side = north ? y_cell : y_cell + 1;

				p_x -= diff_x;
				p_y -= diff_x * slope;
				p_x += ((y_cell_side * CELLSIZE ) - p_y) / slope;
				p_y = float(y_cell_side * CELLSIZE);

				cell_orientation = north ? 1 : 3;

				PushDrawSide(cur_side, p_x, p_y, cell_orientation);
				if (!(cur_side->flags & TRANSLUCENT))
					goto drawcol;
				else
				{
					// Continue
					p_x = p_x_old;
					p_y = p_y_old;
				}
			}
		}

		// Now look horizontally
		grid_y = new_grid_y;
		if (++cell_distance >= maxDistance)
			goto drawcol;
		if (grid_x < 0 || grid_x >= map.boundsX)
			goto drawcol;
		offset = (map.boundsX * grid_y) + grid_x;
		cur_side = east ? &cellptr[offset].e : &cellptr[offset].w;
		if (cur_side->type)
		{
			cell_orientation = east ? 0 : 2;
			PushDrawSide(cur_side, p_x, p_y, cell_orientation);
			if (!(cur_side->flags & TRANSLUCENT))
				goto drawcol;
		}

		// Didn't find anything
		diff_x = east ? float(CELLSIZE) : float(-CELLSIZE);
		grid_x += east ? 1 : -1;
		goto trace;

	drawcol:
		while (draw_side_stack) 
		{
			DrawSide* ds = draw_side_stack;
			DrawColumn(target, ds, angle, col);
			draw_side_stack = draw_side_stack->next;
			SDL_free(ds);
		}
	}
}

static void DrawColumn(SDL_Surface* target, const DrawSide* ds, float angle, u16 col)
{
	// For doors, we basically have to work out how open the door is (presumably as a ratio between
	// total height and pixel height, and what pixel to start dawing at
	// -> this is gonna become spaghetti code super fast.

	// Do triangular correction on the distance
	const float relative_angle = TO_RADF(viewport_angle) - angle;
	const float dist = ds->dist * cosf(relative_angle);

	// Get the wall parameters
	i32 wall_h = (i32)(projection_dist * CELLHEIGHT / dist);
	i32 wall_y = (viewport_h - wall_h) / 2;

	// Get the texture
	const u8 tx_index = ds->side->type & 0x00FF;
	u8 tx_sheet = (ds->side->type & 0xFF00) >> 8;
	if (tx_sheet >= fillCount) 
		tx_sheet = 0;

	const u16 tex_x = ds->texcol + (tx_index & 0x0F) * TEX_SIZE;
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

	if (ds->side->flags & DOOR_V)
	{
		float offset = 0.;
		if (ds->side->door.status & 1)
			offset = (float)ds->side->door.timer_ticks /
			( ds->side->door.status & 2 ? -ds->side->door.closespeed : ds->side->door.openspeed );

		tex_y_px += ds->side->door.scroll + offset;

		wall_h = (i32)(projection_dist * (CELLHEIGHT - ds->side->door.scroll) / dist);
		y_end = SDL_min(viewport_h, wall_y + wall_h);
	}

	u32* render_px = (u32*)target->pixels;
	float* z_buffer_px = z_buffer;

	render_px += (y_top * viewport_w) + col;
	z_buffer_px += (y_top * viewport_w) + col;

	for (i32 draw_y = y_top; draw_y < y_end; ++draw_y )
	{
		tex_px = tx + (u8)tex_y_px * TEX_MAP_SIZE;
		const u32 tex_col = *tex_px;
		if (tex_col != COLOR_KEY)
		{
			*render_px = tex_col;
			*z_buffer_px = dist;
		}

		render_px += viewport_w;
		z_buffer_px += viewport_w;
		tex_y_px += inc_ratio;
	}
}
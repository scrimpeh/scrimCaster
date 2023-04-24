#include "renderutil.h"

#include <math.h>

static const u8 CLIP_L = 0x1;
static const u8 CLIP_R = 0x2;
static const u8 CLIP_U = 0x4;
static const u8 CLIP_D = 0x8;

// Temporary function, to quickly get something visible on screen
// should be replaced by a proper solution
void r_draw_text(SDL_Surface* target, const char* str, i32 x, i32 y, TTF_Font* font, cm_color color)
{
	if (!target || !font)
		return;

	SDL_Rect r = { x, y, 0, 0 };
	SDL_Color sdl_color;
	cm_to_sdl_color(&sdl_color, color);
	SDL_Surface* text = TTF_RenderText_Solid(font, str, sdl_color);
	SDL_BlitSurface(text, NULL, target, &r);
	SDL_FreeSurface(text);
}

void r_draw_pixel(SDL_Surface* target, i32 x, i32 y, cm_color color)
{
	if (x >= 0 && x < target->w && y >= 0 && y < target->h)
		*((u32*)target->pixels + y * target->w + x) = color;
}

// Draw a line using Bresenham's algorithm
void r_draw_line(SDL_Surface* target, i32 x_a, i32 y_a, i32 x_b, i32 y_b, cm_color color)
{
	if (!r_clip_line(target->w - 1, target->h - 1, &x_a, &y_a, &x_b, &y_b))
		return;

	i32 temp;
	const bool steep = abs(y_b - y_a) > abs(x_b - x_a);

	// Swap x1 and y1, swap x2 and y2
	if (steep)
	{
		temp = y_a;
		y_a = x_a;
		x_a = temp;

		temp = y_b;
		y_b = x_b;
		x_b = temp;
	}

	// Swap x1 and x2, swap y1 and y2
	if (x_a > x_b)
	{
		temp = x_b;
		x_b = x_a;
		x_a = temp;

		temp = y_b;
		y_b = y_a;
		y_a = temp;
	}

	const i32 dx = x_b - x_a;
	const i32 dy = abs(y_b - y_a);

	i32 err = dx / 2;
	const i32 y_step = (y_a < y_b) ? 1 : -1;

	i32 y = y_a;
	for (i32 x = x_a; x <= x_b; ++x)
	{
		const i32 x_draw = steep ? y : x;
		const i32 y_draw = steep ? x : y;
		SDL_assert(x_draw >= 0 && x_draw < target->w && y_draw >= 0 && y_draw < target->h);
		*((u32*)target->pixels + y_draw * target->w + x_draw) = color;

		err -= dy;
		if (err < 0)
		{
			y += y_step;
			err += dx;
		}
	}
};

// Clips a line into the rectangle using the
// Cohen-Sutherland algorithm, returns whether to draw the line
static bool r_clip_line(i32 x_max, i32 y_max, i32* x_a, i32* y_a, i32* x_b, i32* y_b)
{
	u8 clip_a = r_clip_code(x_max, y_max, *x_a, *y_a);
	u8 clip_b = r_clip_code(x_max, y_max, *x_b, *y_b);

	while (true) 
	{
		if (!(clip_a | clip_b))
			return true;
		else if (clip_a & clip_b)
			return false;
		else
		{
			i32 x_new;
			i32 y_new;

			const i32 dx = *x_b - *x_a;
			const i32 dy = *y_b - *y_a;
			const u8 clip_out = clip_b > clip_a ? clip_b : clip_a;

			if (clip_out & CLIP_D) 
			{
				x_new = *x_a + dx * (y_max - *y_a) / dy;
				y_new = y_max;
			}
			else if (clip_out & CLIP_U) 
			{
				x_new = *x_a + dx * -*y_a / dy;
				y_new = 0;
			}
			else if (clip_out & CLIP_R) 
			{
				y_new = *y_a + dy * (x_max - *x_a) / dx;
				x_new = x_max;
			}
			else if (clip_out & CLIP_L) 
			{
				y_new = *y_a + dy * -*x_a / dx;
				x_new = 0;
			}

			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (clip_out == clip_a) {
				*x_a = x_new;
				*y_a = y_new;
				clip_a = r_clip_code(x_max, y_max, *x_a, *y_a);
			}
			else {
				*x_b = x_new;
				*y_b = y_new;
				clip_b = r_clip_code(x_max, y_max, *x_b, *y_b);
			}
		}
	}
}

static inline u8 r_clip_code(i32 x_max, i32 y_max, i32 x, i32 y)
{
	u8 code = 0;
	if (x < 0) 
		code |= CLIP_L;
	if (x > x_max) 
		code |= CLIP_R;
	if (y < 0) 
		code |= CLIP_U;
	if (y > y_max) 
		code |= CLIP_D;
	return code;
}


#include <render/viewport.h>

#include <game/camera.h>
#include <render/render.h>
#include <util/mathutil.h>

#include <float.h>
#include <math.h>

u16 viewport_w = 256;
u16 viewport_h = 192;
u8 viewport_x_fov = 90;

SDL_Surface* viewport_surface;
float* viewport_z_buffer = NULL;

float viewport_projection_distance = 0;

i32 viewport_init(u16 w, u16 h)
{
	viewport_destroy();

	viewport_w = w;
	viewport_h = h;

	viewport_surface = SDL_CreateRGBSurface(0, viewport_w, viewport_h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000);
	if (!viewport_surface)
	{
		viewport_destroy();
		return -1;
	}

	viewport_z_buffer = SDL_malloc(sizeof(float) * viewport_w * viewport_h);
	if (!viewport_z_buffer)
	{
		viewport_destroy();
		return -1;
	}
	for (u32 i = 0; i < viewport_w * viewport_h; i++)
		viewport_z_buffer[i] = FLT_MAX;

	viewport_set_fov(viewport_x_fov);
	return 0;
}

void viewport_set_fov(u8 fov)
{
	viewport_x_fov = MATH_CAP(VIEWPORT_FOV_MIN, fov, VIEWPORT_FOV_MAX);
	viewport_projection_distance = (viewport_w / 2) / tanf(TO_RADF(viewport_x_fov / 2));
}

void viewport_destroy()
{
	SDL_FreeSurface(viewport_surface);
	viewport_surface = NULL;

	SDL_free(viewport_z_buffer);
	viewport_z_buffer = NULL;
}

i32 viewport_angle_to_x(angle_rad_f angle)
{
	return tanf(angle) * viewport_projection_distance + (viewport_w / 2);
}

i32 viewport_distance_to_length(i32 size, float distance)
{
	return (viewport_projection_distance * size) / distance;
}

i32 viewport_distance_to_y(float distance, i32 z)
{
	return (viewport_h / 2) - viewport_distance_to_length(R_HALF_H, distance) + viewport_distance_to_length(z, distance);
}

angle_rad_f viewport_x_to_angle(angle_rad_f viewport_angle, i32 x)
{
	const i32 x_offset = x - (viewport_w / 2);
	return angle_normalize_rad_f(viewport_angle - atanf(x_offset / viewport_projection_distance));
}

float viewport_y_to_distance(i32 y)
{
	const i32 h = abs(viewport_h - (2 * y));
	return (viewport_projection_distance * R_CELL_H) / h;
}

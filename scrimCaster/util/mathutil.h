#pragma once

#include <common.h>

typedef struct
{
	float x;
	float y;
} math_vec_2f;

typedef struct
{
	double x;
	double y;
} math_vec_2d;

#define MATH_DIVCEIL(a, b) (((a) / (b)) + ((a) % (b) ? 1 : 0))

#define MATH_CAP(min, value, max) (SDL_max(min, SDL_min(value, max)))

double math_dist(double a_x, double a_y, double b_x, double b_y);
float math_dist_f(float a_x, float a_y, float b_x, float b_y);

math_vec_2d math_vec_cast(double x, double y, angle_rad_d angle, double d);
math_vec_2f math_vec_cast_f(float x, float y, angle_rad_f angle, float d);

float math_lerp(float x0, float x1, float x, float y0, float y1);
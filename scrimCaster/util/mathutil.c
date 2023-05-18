#include <util/mathutil.h>

#include <math.h>

double math_dist(double a_x, double a_y, double b_x, double b_y)
{
	return sqrt(pow(a_x - b_x, 2) + pow(a_y - b_y, 2));
}

float math_dist_f(float a_x, float a_y, float b_x, float b_y)
{
	return sqrtf(powf(a_x - b_x, 2) + powf(a_y - b_y, 2));
}

math_vec_2d math_vec_cast(double x, double y, angle_rad_d angle, double d)
{
	math_vec_2d result;
	const double arctan = angle < M_PI ? angle : (-PI_2_1 + angle);
	result.x = x + d * SDL_cos(arctan);
	result.y = y + d * -SDL_sin(arctan);
	return result;
}

math_vec_2f math_vec_cast_f(float x, float y, angle_rad_f angle, float d)
{
	math_vec_2f result;
	const float arctan = angle < M_PI ? angle : (-PI_2_1 + angle);
	result.x = x + d * SDL_cosf(arctan);
	result.y = y + d * -SDL_sinf(arctan);
	return result;
}

float math_lerp(float x0, float x1, float x, float y0, float y1)
{
	const float x_range = x1 - x0;
	const float y_range = y1 - y0;
	const float diff = x - x0;
	const float ratio = diff / x_range;
	return y0 + (ratio * y_range);
}
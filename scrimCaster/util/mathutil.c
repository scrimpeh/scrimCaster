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

void math_vec_cast(double x, double y, angle_rad_d angle, double d, double* result_x, double* result_y)
{
	const double arctan = angle < M_PI ? angle : (-PI_2_1 + angle);

	const double dx = d * SDL_cos(arctan);
	const double dy = d * SDL_sin(arctan) * -1;

	*result_x = x + dx;
	*result_y = y + dy;
}

void math_vec_cast_f(float x, float y, angle_rad_f angle, float d, float* result_x, float* result_y)
{
	const float arctan = angle < M_PI ? angle : (-PI_2_1 + angle);

	const float dx = d * SDL_cos(arctan);
	const float dy = d * SDL_sin(arctan) * -1;

	*result_x = x + dx;
	*result_y = y + dy;
}

float math_lerp(float x0, float x1, float x, float y0, float y1)
{
	const float x_range = x1 - x0;
	const float y_range = y1 - y0;
	const float diff = x - x0;
	const float ratio = diff / x_range;
	return y0 + (ratio * y_range);
}
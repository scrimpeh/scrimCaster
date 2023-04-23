#include "mathutil.h"

#include <math.h>

double math_dist(double a_x, double a_y, double b_x, double b_y)
{
	return sqrt(pow(a_x - b_x, 2) + pow(a_y - b_y, 2));
}

float math_dist_f(float a_x, float a_y, float b_x, float b_y)
{
	return sqrtf(powf(a_x - b_x, 2) + powf(a_y - b_y, 2));
}
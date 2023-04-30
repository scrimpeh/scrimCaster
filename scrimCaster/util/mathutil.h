#pragma once

#include <common.h>

#define DIVCEIL(a, b) (((a) / (b)) + ((a) % (b) ? 1 : 0))

double math_dist(double a_x, double a_y, double b_x, double b_y);
float math_dist_f(float a_x, float a_y, float b_x, float b_y);

void math_vec_cast(double x, double y, angle_rad_d angle, double d, double* result_x, double* result_y);
void math_vec_cast_f(float x, float y, angle_rad_f angle, float d, float* result_x, float* result_y);
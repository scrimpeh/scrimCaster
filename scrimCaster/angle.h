#pragma once

// Definition for angles. Angles exist either as degrees, in the range [0, 360,
// going counter-clockwise, with 0 facing east. or as radians, in the range [0, 2 * M_PI[
//
//           [ 90 | PI/2 ]
//                 ^
//                 |
// [ 180 | PI ] <- O -> [ 0 | 0 ]
//                 |
//                 v
//          [ 270 | PI*3/2 ]

#include <common.h>

#include <math.h>

typedef float angle_f;
typedef double angle_d;
typedef float angle_rad_f;
typedef double angle_rad_d;

#define PI_1_2 (M_PI / 2.)
#define PI_1_4 (M_PI / 4.)
#define PI_3_4 (3 * PI_1_4)
#define PI_3_2 (3 * PI_1_2)
#define PI_2_1 (M_PI * 2)

#define TO_RAD(a) ((a) * (M_PI / 180))
#define TO_DEG(a) ((a) * (180 / M_PI))

#define TO_RADF(a) ((a) * (float)(M_PI / 180))
#define TO_DEGF(a) ((a) * (float)(180 / M_PI))

angle_rad_f angle_normalize_rad_f(angle_rad_f angle);
angle_rad_d angle_normalize_rad_d(angle_rad_d angle);
angle_f angle_normalize_deg_f(angle_f angle);
angle_d angle_normalize_deg_d(angle_d angle);

angle_rad_f angle_get_rad_f(float dx, float dy);
angle_rad_d angle_get_rad_d(double dx, double dy);
angle_f angle_get_deg_f(float dx, float dy);
angle_d angle_get_deg_d(double dx, double dy);
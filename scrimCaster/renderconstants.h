#pragma once
#include "types.h"

/* Constant defines that need to be in more than one header,
e.g. array bounds and macros.*/

#define PI_1_2 (M_PI / 2.)
#define PI_1_4 (M_PI / 4.)
#define PI_3_4 (3*PI_1_4)
#define PI_3_2 (3*PI_1_2)
#define PI_2_1 (M_PI * 2)
	
#define TO_RAD(a) ((a) * (M_PI / 180))
#define TO_DEG(a) ((a) * (180 / M_PI))

#define TO_RADF(a) ((a) * float(M_PI / 180))
#define TO_DEGF(a) ((a) * float(180 / M_PI))

/*
arctan = TO_RAD(actor->angle < 180 ?
actor->angle :
-360 + actor->angle);*/

#define ATAN_INTERNAL(a, halfcircle, circle) ((a) < (halfcircle) ? (a) : (-(circle)) + (a))
#define ATAN_RAD(a) ATAN_INTERNAL(a, M_PI, M_PI*2)
#define ATAN_DEG(a) ATAN_INTERNAL(a, 180, 360)

#define TEX_SIZE 64
#define TEX_MAP_SIZE 1024

const u32 COLOR_KEY = 0xFF00FF;

extern inline float AngleToDeg(float x1, float y1, float x2, float y2);
extern inline double AngleToDeg(double x1, double y1, double x2, double y2);

extern inline float AngleToDeg(float y_disp, float x_disp);
extern inline double AngleToDeg(double y_disp, double x_disp);

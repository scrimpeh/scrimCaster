#pragma once

#include <common.h>

#include <colormap.h>

#define ATAN_INTERNAL(a, halfcircle, circle) ((a) < (halfcircle) ? (a) : (-(circle)) + (a))
#define ATAN_RAD(a) ATAN_INTERNAL(a, M_PI, M_PI * 2)
#define ATAN_DEG(a) ATAN_INTERNAL(a, 180, 360)

#define TEX_SIZE 64
#define TEX_MAP_SIZE 1024

const cm_color COLOR_KEY = CM_GET(255, 0, 255);

extern inline float AngleToDeg(float x1, float y1, float x2, float y2);
extern inline double AngleToDeg(double x1, double y1, double x2, double y2);

extern inline float AngleToDeg(float y_disp, float x_disp);
extern inline double AngleToDeg(double y_disp, double x_disp);

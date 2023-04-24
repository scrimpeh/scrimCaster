#pragma once

#include "common.h"

extern u8 viewport_x_fov;
extern float viewport_angle;
extern float viewport_x;
extern float viewport_y;

void SetViewportFov(u8 fov);
void UpdateCamera(u32 delta);
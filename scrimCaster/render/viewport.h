#pragma once

// This module unifies everything related to the viewport. The viewport is initialized
// with certain properties here, and destroyed here. It provides primitives for 
// translating a pixel position on screen to an angle and distance and vice versa.

// This module also handles the Z-buffer, and all other ancillary data structures I might need

#include <common.h>

#include <SDL/SDL_video.h>

#define VIEWPORT_FOV_MIN 20
#define VIEWPORT_FOV_MAX 150

extern u16 viewport_w;
extern u16 viewport_h;
extern u8 viewport_x_fov;

// The projection distance is the distance at which the size of pixels on screen is as large as the distance in map units
extern float viewport_projection_distance;

// The main drawing surface
extern SDL_Surface* viewport_surface;

// The Z Buffer is used for masking sprites against the wall. It covers every pixel on the screen to support blending sprites
// against transparent walls
extern float* viewport_z_buffer;


i32 viewport_init(u16 w, u16 h);
void viewport_set_fov(u8 fov);
void viewport_destroy();

i32 viewport_angle_to_x(angle_rad_f angle);
// z = distance from the top
i32 viewport_distance_to_y(float distance, i32 z);
i32 viewport_distance_to_length(i32 size, float distance);

angle_rad_f viewport_x_to_angle(angle_rad_f viewport_angle, i32 x);
float viewport_y_to_distance(i32 y);
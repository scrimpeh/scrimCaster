#pragma once

// A color ramp is a sequence of colors which are alpha blended into the render colors
// at a given distance. This is used to the light level of rooms dropping off, 
// as well as e.g., distance fog. Between each set point, colors are interpolated linearly.

// There is one global color ramp for the current level, which is defined by the map,
// but it is conceivable that the color ramp is changed dynamically through a trigger.

// For any color ramp, there must always be at least one color at a distance <= 0,
// which serves as the base color, and a color at FLT_MAX, which serves as the furthest most color

// Color ramps should generally be allocated on the heap, and freed when they are no longer used

#include <common.h>

#include <render/color/colormap.h>

typedef struct
{
	cm_color color;
	float alpha;
	float distance;
} cm_ramp_setpoint;

typedef struct
{
	cm_ramp_setpoint* points;
	u32 size;
} cm_ramp;


cm_ramp* cm_ramp_create(u32 size);
void cm_ramp_free(cm_ramp* ramp);

void cm_ramp_set(cm_ramp* new_ramp);
const cm_ramp* cm_ramp_get();

// Test integrity of color ramp
static bool cm_ramp_valid(const cm_ramp* ramp);

cm_color cm_ramp_mix(cm_color color, float distance);
cm_color cm_ramp_mix_infinite(cm_color color);

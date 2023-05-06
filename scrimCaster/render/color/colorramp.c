#include <render/color/colorramp.h>

#include <util/mathutil.h>

#include <float.h>

static const cm_ramp_setpoint CM_RAMP_NONE_PTS[] =
{
	{ CM_GET(0, 0, 0), 0.f,     0.f},
	{ CM_GET(0, 0, 0), 0.f, FLT_MAX},
};
static const cm_ramp CM_RAMP_NONE = { CM_RAMP_NONE_PTS, SDL_arraysize(CM_RAMP_NONE_PTS) };

static const cm_ramp_setpoint CM_RAMP_TEST_PTS[] =
{
	{ CM_GET(   0,    0,    0),   0.f,   -128.f},
	{ CM_GET(   0,    0, 0xFF),  0.1f,    128.f},
	{ CM_GET(0xFF,    0, 0xFF),  1.0f,    256.f},
	{ CM_GET(   0,    0, 0xFF), 0.35f,    768.f},
	{ CM_GET(   0,    0, 0xFF), 0.35f,  FLT_MAX},
};
static const cm_ramp CM_RAMP_TEST = { CM_RAMP_TEST_PTS, SDL_arraysize(CM_RAMP_TEST_PTS) };

static const cm_ramp_setpoint CM_RAMP_DARK_PTS[] =
{
	{ CM_GET(   0,    0,    0),    0.f,     0.f},
	{ CM_GET(   0,    0,    0),    0.f,     0.f},
	{ CM_GET(   0,    0,    0),  0.95f,   256.f},
	{ CM_GET(   0,    0,    0),  0.95f, FLT_MAX},
};
static const cm_ramp CM_RAMP_DARK = { CM_RAMP_DARK_PTS, SDL_arraysize(CM_RAMP_DARK_PTS) };

static const cm_ramp_setpoint CM_RAMP_TWILIGHT_PTS[] =
{
	{ CM_GET(   0,    0,    0),    0.f,     0.f},
	{ CM_GET(   0,    0,    0),    0.f,     0.f},
	{ CM_GET(   0,    0,    0),   0.6f,   256.f},
	{ CM_GET(   0,    0,    0),   0.6f, FLT_MAX},
};
static const cm_ramp CM_RAMP_TWILIGHT = { CM_RAMP_TWILIGHT_PTS, SDL_arraysize(CM_RAMP_TWILIGHT_PTS) };

static const cm_ramp_setpoint CM_RAMP_FOG_PTS[] =
{
	{ CM_GET(   0,    0,    0),    0.f,     0.f},
	{ CM_GET(   0,    0,    0),    0.f,     0.f},
	{ CM_GET(0x70, 0x60, 0x30),   0.3f,   192.f},
	{ CM_GET(0x70, 0x60, 0x30),   0.3f, FLT_MAX},
};
static const cm_ramp CM_RAMP_FOG = { CM_RAMP_FOG_PTS, SDL_arraysize(CM_RAMP_FOG_PTS) };

cm_ramp* cm_ramp_current = &CM_RAMP_FOG;

cm_ramp* cm_ramp_create(u32 size)
{
	cm_ramp_setpoint* setpoints = SDL_malloc(sizeof(cm_ramp_setpoint) * size);
	if (!setpoints)
		return NULL;
	cm_ramp* ramp = SDL_malloc(sizeof(cm_ramp));
	if (!ramp)
	{
		SDL_free(setpoints);
		return NULL;
	}
	ramp->size = size;
	ramp->points = setpoints;
	return ramp;
}

void cm_ramp_free(cm_ramp* ramp)
{
	SDL_free(ramp->points);
	SDL_free(ramp);
}

void cm_ramp_set(cm_ramp* new_ramp)
{
	SDL_assert(cm_ramp_valid(new_ramp));
	cm_ramp_current = new_ramp;
}

const cm_ramp* cm_ramp_get()
{
	return cm_ramp_current;
}

static bool cm_ramp_valid(const cm_ramp* ramp)
{
	if (ramp->size <= 1)
		return false;
	if (ramp->points[0].distance > 0)
		return false;
	if (ramp->points[ramp->size - 1].distance != FLT_MAX)
		return false;
	float distance = -FLT_MAX;
	for (u32 i = 0; i < ramp->size; i++)
	{
		if (ramp->points[i].distance < distance)
			return false;
		distance = ramp->points[i].distance;
	}
}

cm_color cm_ramp_mix(cm_color color, float distance)
{
	// Find the nearest two setpoints. Linear search is okay for this, we only need to do this every pixel (lol)
	u32 i = 0;
	while (cm_ramp_current->points[i].distance <= distance)
		i++;

	// Linearly interpolate the color and the distance between the setpoints
	const cm_ramp_setpoint* pt_a = &cm_ramp_current->points[i - 1];
	const cm_ramp_setpoint* pt_b = &cm_ramp_current->points[i];

	const float alpha = math_lerp(pt_a->distance, pt_b->distance, distance, pt_a->alpha, pt_b->alpha);
	const u8 r = (u8) math_lerp(pt_a->distance, pt_b->distance, distance, CM_R(pt_a->color), CM_R(pt_b->color));
	const u8 g = (u8) math_lerp(pt_a->distance, pt_b->distance, distance, CM_G(pt_a->color), CM_G(pt_b->color));
	const u8 b = (u8) math_lerp(pt_a->distance, pt_b->distance, distance, CM_B(pt_a->color), CM_B(pt_b->color));
	
	const cm_color mix = CM_GET(r, g, b);
	return cm_map(color, mix, alpha);
}

cm_color cm_ramp_mix_infinite(cm_color color)
{
	const cm_ramp_setpoint* last = &cm_ramp_current->points[cm_ramp_current->size - 1];
	return cm_map(color, last->color, last->alpha);
}
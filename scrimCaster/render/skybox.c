#include <render/skybox.h>

#include <render/color/colorramp.h>
#include <render/render.h>
#include <render/gfxloader.h>

const char* R_SKY_PATH = "tx/sky/";

extern float viewport_angle;

#define R_SKYBOX_COUNT 1

const char* R_SKYBOX_FILENAMES[R_SKYBOX_COUNT] =
{
	"sky_default.png"
};

#define R_SKY_PATH_BUF_SIZE 512
char r_sky_path_buf[R_SKY_PATH_BUF_SIZE];

r_skybox* r_sky_skyboxes = NULL;
u32 r_skyboxes_size = 0;

u32 r_sky_current = 0;

i32 r_sky_load_global()
{
	r_sky_unload();

	r_skyboxes_size = R_SKYBOX_COUNT;
	r_sky_skyboxes = SDL_malloc(sizeof(r_skybox) * r_skyboxes_size);
	if (!r_sky_skyboxes)
		return -1;
	for (u32 i = 0; i < r_skyboxes_size; i++)
		if (r_sky_load(&r_sky_skyboxes[i], R_SKYBOX_FILENAMES[i]))
			return -1;

	return 0;
}

void r_sky_set_current(i32 sky)
{
	r_sky_current = sky;
}

static i32 r_sky_load(r_skybox* sky, const char* path)
{
	r_sky_path_buf[0] = '\0';
	SDL_snprintf(r_sky_path_buf, R_SKY_PATH_BUF_SIZE, "%s%s", R_SKY_PATH, path);
	SDL_Surface* surface = gfx_load(r_sky_path_buf);
	if (!surface)
		return -1;
	sky->w = surface->w;
	sky->h = surface->h;
	sky->data = SDL_malloc(sizeof(u32) * sky->w * sky->h);
	if (!sky->data)
		return -1;
	tx_copy(surface, NULL, sky->data);
	SDL_FreeSurface(surface);

	return 0;
}

void r_sky_unload()
{
	for (u32 i = 0; i < r_skyboxes_size; i++)
		SDL_free(r_sky_skyboxes[i].data);
	SDL_free(r_sky_skyboxes);
	r_skyboxes_size = 0;
}

void r_sky_draw(SDL_Surface* target, u16 col, u16 y_start, u16 y_end, angle_rad_f angle)
{
	for (u32 y = y_start; y < y_end; y++)
		*(((u32*) target->pixels) + y * target->w + col) = r_sky_get_pixel(y, angle);
}

u32 r_sky_get_pixel(u16 y, angle_rad_f angle)
{
	// TODO: I am not sure if I want to undo the theta correction here.
	const r_skybox* sky = &r_sky_skyboxes[r_sky_current];

	const u16 x_sky = ((float) angle / PI_2_1) * sky->w;
	const u16 y_sky = ((float) y / viewport_h) * sky->h;

	const u32 px = *(sky->data + x_sky * sky->h + y_sky);
	return cm_ramp_mix_infinite(px);
}
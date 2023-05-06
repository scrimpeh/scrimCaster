#include <game/camera.h>

#include <game/actor/actor.h>
#include <input/input.h>
#include <render/render.h>
#include <render/scan.h>

const u8 VIEWPORT_FOV_MIN = 20;
const u8 VIEWPORT_FOV_MAX = 170;
extern Actor player;

float viewport_angle;
float viewport_x;
float viewport_y;

void SetViewportFov(u8 fov)
{
	if (VIEWPORT_FOV_MIN < fov && fov < VIEWPORT_FOV_MAX)
		viewport_x_fov = fov;
}
	
void UpdateCamera(u32 delta)
{
	viewport_angle = (float) player.angle;
	viewport_x = (float) player.x;
	viewport_y = (float) player.y;
}
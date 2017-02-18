#include "types.h"
#include "camera.h"

#include "actor.h"
#include "scan.h"

const u8 VIEWPORT_FOV_MIN = 20;
const u8 VIEWPORT_FOV_MAX = 170;
extern Actor player;

u8 viewport_x_fov;
float viewport_angle;
float viewport_x, viewport_y;

void SetViewportFov(u8 fov)
{
	if (VIEWPORT_FOV_MIN < fov && fov < VIEWPORT_FOV_MAX)
		viewport_x_fov = fov;
}
	
void UpdateCamera(u32 delta)
{
	viewport_angle = (float)player.angle;
	viewport_x = (float)player.x;
	viewport_y = (float)player.y;
}
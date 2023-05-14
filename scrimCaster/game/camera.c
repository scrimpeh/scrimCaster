#include <game/camera.h>

#include <game/gameobjects.h>
#include <input/input.h>
#include <render/render.h>
#include <render/scan.h>

angle_f viewport_angle;
float viewport_x;
float viewport_y;

static ac_actor* cam_actor = NULL;

void cam_set_actor(ac_actor* actor)
{
	cam_actor = actor;
}

ac_actor* cam_get_actor()
{
	return cam_actor;
}
	
void cam_update(u32 delta)
{
	const ac_actor* viewport_actor = cam_get_actor();

	if (!viewport_actor)
	{
		viewport_angle = 315;
		viewport_x = 0;
		viewport_y = 0;
	}
	else
	{
		viewport_angle = (angle_f) viewport_actor->angle;
		viewport_x = (float) viewport_actor->x;
		viewport_y = (float) viewport_actor->y;
	}


}
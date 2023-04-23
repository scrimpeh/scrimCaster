#include "angle.h"

angle_rad_f angle_normalize_rad_f(angle_rad_f angle)
{
	while (angle < 0)
		angle += PI_2_1;
	while (angle >= PI_2_1)
		angle -= PI_2_1;
	return angle;
}

angle_rad_d angle_normalize_rad_d(angle_rad_d angle)
{
	while (angle < 0)
		angle += PI_2_1;
	while (angle >= PI_2_1)
		angle -= PI_2_1;
	return angle;
}

angle_f angle_normalize_deg_f(angle_f angle)
{
	while (angle < 0)
		angle += 360;
	while (angle >= 360)
		angle -= 360;
	return angle;
}

angle_d angle_normalize_deg_d(angle_d angle)
{
	while (angle < 0)
		angle += 360;
	while (angle >= 360)
		angle -= 360;
	return angle;
}
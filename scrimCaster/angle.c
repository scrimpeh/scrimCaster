#include <angle.h>

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

angle_rad_f angle_get_rad_f(float dx, float dy)
{
	return atan2f(dy, dx);
}

angle_rad_d angle_get_rad_d(double dx, double dy)
{
	return atan2(dy, dx);
}

angle_f angle_get_deg_f(float dx, float dy)
{
	return angle_normalize_deg_f(TO_DEGF(angle_get_rad_f(dx, dy)));
}

angle_d angle_get_deg_d(double dx, double dy)
{
	return angle_normalize_deg_d(TO_DEG(angle_get_rad_f(dx, dy)));
}

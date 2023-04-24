#include "renderconstants.h"

#include <math.h>

/* Take the angle between two points and return 
   them as an angle between 0 and 360 degrees */

inline float AngleToDeg(float x1, float y1, float x2, float y2)
{
	return AngleToDeg(y2 - y1, x2 - x1);
}

inline double AngleToDeg(double x1, double y1, double x2, double y2)
{
	return AngleToDeg(y2 - y1, x2 - x1);
}

inline float AngleToDeg(float y_disp, float x_disp)
{
	const float a = TO_DEGF(atan2f(y_disp, x_disp));
	return a >= 0 ? a : 360 + a;
}

inline double AngleToDeg(double y_disp, double x_disp)
{
	const double a = TO_DEG(atan2(y_disp, x_disp));
	return a >= 0 ? a : 360 + a;
}

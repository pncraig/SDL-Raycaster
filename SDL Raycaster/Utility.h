#ifndef UTILITY_H
#define UTILITY_H

#include "SDL.h"

namespace util
{
	// Convert an angle from degrees to radians (needed for use with trigonometric functions)
	double radians(double degrees);

	// Convert an angle from radians to degrees
	double degrees(double radians);

	// Loop angles to fit on the interval of 0 - 360 degrees
	double getCoterminalAngle(double angle);

	// Restrict a value between a minimum value and a maximum value
	double clamp(double x, double lo, double hi);

	double distance(double aX, double aY, double bX, double bY);

	uint32_t calculateLighting(const uint32_t& color, double lighting);
}

#endif

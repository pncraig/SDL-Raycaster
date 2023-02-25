#include "Utility.h"

double util::radians(double degrees)
{
	return degrees * (M_PI / 180.0);
}

double util::degrees(double radians)
{
	return radians * (180.0f / M_PI);
}

double util::getCoterminalAngle(double angle)
{
	if (angle < 0.0f)
	{
		while (angle < 0)
		{
			angle += 360.0f;
		}
		return angle;
	}
	else if (angle >= 360.0f)
		return angle - (static_cast<int>(angle / 360.0) * 360);
	else return angle;
}

double util::clamp(double x, double lo, double hi)
{
	if (x > hi)
		return hi;

	if (x < lo)
		return lo;

	return x;
}

uint32_t util::calculateLighting(const uint32_t& color, double lighting)
{
	uint32_t red{ color >> 24 };
	uint32_t green{ (color >> 16) - (red << 8) };
	uint32_t blue{ (color >> 8) - (red << 16) - (green << 8) };

	// Calculate the brightness of each color according to the lighting
	// (0.0039215686 is the inverse of 255)
	red = static_cast<uint32_t>(red * 0.0039215686f * lighting);
	green = static_cast<uint32_t>(green * 0.0039215686f * lighting);
	blue = static_cast<uint32_t>(blue * 0.0039215686f * lighting);

	// Move the compontents to their original hex positions
	red <<= 24;
	green <<= 16;
	blue <<= 8;

	// Create a new color from the components and output it to the screen
	return uint32_t{ red + green + blue + 0x000000FF };
}
#ifndef RAY_H
#define RAY_H

#include "vec2.h"

class Ray
{
private:
	float m_x;
	float m_y;
	float m_dir;
public:
	Ray(float x, float y, float dir);

	vec2 cast(const std::string& map, int mapWidth, int mapHeight, int gridSize);
};

#endif


#include "Ray.h"

float getCoterminalAngle(float angle);
float degrees(float radians);
float radians(float degrees);

Ray::Ray(float x, float y, float dir)
	:m_x{ x }, m_y{ y }, m_dir{ dir }
{

}

vec2 Ray::cast(const std::string& map, int mapWidth, int mapHeight, int gridSize)
{
	// Precalculate the value of tan of rayAngle because that value is used eight times
	float tanOfRayAngle{ tanf(radians(m_dir)) };

	// Find the angle of the ray on the interval 0 <= rayAngle < 360
	m_dir = getCoterminalAngle(m_dir);

	if (m_dir == 360.0f)
		m_dir = 0.0f;

	// CALCULATE HORIZONTAL INTERSECTIONS
	float horizontalIntersectionsDistance{ -1.0f };

	// Point A is the point of the first intersection between the ray and the horizontal grid lines
	float aX{};
	float aY{};

	// The change between point A and the next intersection with the horizontal grid lines
	float dx{};
	float dy{};

	// If the ray is facing up... (or downwards on the coordinate grid)
	if (m_dir < 180)
	{
		// The first intersection will be part of the grid below (calculates y-coordinate of grid line below)
		aY = floorf(m_y / static_cast<float>(gridSize)) * gridSize;

		// The next intersection with a horizontal grid line will be gridSize units below
		dy = -static_cast<float>(gridSize);

		//	      90					90		
		//  -x,-y | +x,-y			-tan | +tan
		// 180 ---+--- 0		  180 ---+--- 0	
		//	-x,+y |	+x,+y			+tan | -tan
		//		 270				    270		
		// When rayAngle < 90, dx should be >0, and when rayAngle > 90, dx should be <0
		// It just so happens that tan is >0 when rayAngle < 90 degrees, and tan is <0 when rayAngle > 90
		// so I don't have to change the signs at all
		dx = gridSize / tanOfRayAngle;

		// Calculate the x-coordinate of the first intersection with a horizontal gridline
		aX = m_x - (aY - m_y) / tanOfRayAngle;

		// Make part of the grid below for ease of checking for a wall
		aY--;
	}
	// If ray is facing down... (or upwards on the coordinate grid)
	else
	{
		// The first horizontal grid intersection is with the grid line above the player
		aY = floorf(m_y / static_cast<float>(gridSize)) * gridSize + gridSize;

		// The next gridline with be gridSize units above the player
		dy = static_cast<float>(gridSize);

		//	      90					90		
		//  -x,-y | +x,-y			-tan | +tan
		// 180 ---+--- 0		  180 ---+--- 0	
		//	-x,+y |	+x,+y			+tan | -tan
		//		 270				    270		
		// When rayAngle < 270, dx should be <0, and when rayAngle > 270, dx should be >0
		// It just so happens that tan is >0 when rayAngle < 270 degrees, and tan is <0 when rayAngle > 270
		// so I have to flip the signs with the negative
		dx = -gridSize / tanOfRayAngle;

		// Calculate the x-coordinate of the first intersection with a horizontal gridline
		aX = m_x - (aY - m_y) / tanOfRayAngle;
	}

	// Grid coordinates of point A
	int aXgrid{ static_cast<int>(aX / gridSize) };
	int aYgrid{ static_cast<int>(aY / gridSize) };

	// So long as the x-coordinate in terms of the grid of A is within the bounds of the map...
	if (!(aXgrid < 0 || aXgrid >= mapWidth))
	{
		// If there is a wall in that grid, calculate the distance
		if (map[aYgrid * mapWidth + aXgrid] == '#')
		{
			horizontalIntersectionsDistance = (m_x - aX) * (m_x - aX) + (m_y - aY) * (m_y - aY);
		}
	}
	// If the x-coordinate in terms of the grid is outside the map, ignore it
	else
	{
		horizontalIntersectionsDistance = FLT_MAX;
	}

	// Until a wall has been found and a distance can be calculated...
	while (horizontalIntersectionsDistance < 0.0f)
	{
		// Find next intersection with a horizontal grid line
		aX += dx;
		aY += dy;

		// Convert back to grid coordinates
		aXgrid = static_cast<int>(aX / gridSize);
		aYgrid = static_cast<int>(aY / gridSize);

		// Find the distance
		if (!(aXgrid < 0 || aXgrid >= mapWidth))
		{
			if (map[aYgrid * mapWidth + aXgrid] == '#')
			{
				// It isn't necessary to square root the distance because I am only using this variable to compare to the vertical distance
				horizontalIntersectionsDistance = (m_x - aX) * (m_x - aX) + (m_y - aY) * (m_y - aY);
			}
		}
		else
		{
			horizontalIntersectionsDistance = FLT_MAX;
		}
	}

	// CALCULATE VERTICAL INTERSECTIONS (very similar to calculating horizontal intersections)
	float verticalIntersectionsDistance{ -1.0f };

	// Point B is the point of the first intersection between the ray and the vertical grid lines
	float bX{};
	float bY{};

	// Reset dx and dy
	dx = 0.0f;
	dy = 0.0f;

	// If the ray is facing to the right...
	if (m_dir < 90.0f || m_dir > 270.0f)
	{
		// The first intersection will be in a grid to the right of the current grid
		bX = floorf(m_x / gridSize) * gridSize + gridSize;

		// The ray is moving in a positive x-direction
		dx = static_cast<float>(gridSize);

		//	      90					90		
		//  -x,-y | +x,-y			-tan | +tan
		// 180 ---+--- 0		  180 ---+--- 0	
		//	-x,+y |	+x,+y			+tan | -tan
		//		 270				    270		
		// When rayAngle < 180, dy should be <0, and when rayAngle > 180, dy should be >0
		// It just so happens that tan is >0 when rayAngle < 180 degrees, and tan is <0 when rayAngle > 180
		// so I have to flip the signs with the negative
		dy = -tanOfRayAngle * gridSize;

		// Calculate the y-coordinate of the first intersection with a vertical gridline
		bY = m_y + (m_x - bX) * tanOfRayAngle;
	}
	// If the ray is facing to the left...
	else
	{
		// The first intersection will be in a grid to the left
		bX = floorf(m_x / gridSize) * gridSize;

		// The ray is moving in a negative x-direction
		dx = -static_cast<float>(gridSize);

		//	      90					90		
		//  -x,-y | +x,-y			-tan | +tan
		// 180 ---+--- 0		  180 ---+--- 0	
		//	-x,+y |	+x,+y			+tan | -tan
		//		 270				    270		
		// When rayAngle < 180, dy should be <0, and when rayAngle > 180, dy should be >0
		// It just so happens that tan is <0 when rayAngle < 180 degrees, and tan is >0 when rayAngle > 180
		// so I don't have to change the signs at all
		dy = tanOfRayAngle * gridSize;

		// Calculate the y-coordinate of the first intersection with a vertical gridline
		bY = m_y + (m_x - bX) * tanOfRayAngle;

		bX--;
	}

	// Same process as with the horizontal intersection code
	int bXgrid{ static_cast<int>(bX / gridSize) };
	int bYgrid{ static_cast<int>(bY / gridSize) };

	if (!(bYgrid < 0 || bYgrid >= mapHeight))
	{
		if (map[bYgrid * mapWidth + bXgrid] == '#')
		{
			verticalIntersectionsDistance = (m_x - bX) * (m_x - bX) + (m_y - bY) * (m_y - bY);
		}
	}
	else
	{
		verticalIntersectionsDistance = FLT_MAX;
	}

	while (verticalIntersectionsDistance < 0.0f)
	{
		bX += dx;
		bY += dy;

		bXgrid = static_cast<int>(bX / gridSize);
		bYgrid = static_cast<int>(bY / gridSize);

		if (!(bYgrid < 0 || bYgrid >= mapHeight))
		{
			if (map[bYgrid * mapWidth + bXgrid] == '#')
			{
				verticalIntersectionsDistance = (m_x - bX) * (m_x - bX) + (m_y - bY) * (m_y - bY);
			}
		}
		else
		{
			verticalIntersectionsDistance = FLT_MAX;
		}
	}

	if (verticalIntersectionsDistance < horizontalIntersectionsDistance)
		return vec2{ bX, bY };
	else
		return vec2{ aX, aY };
}
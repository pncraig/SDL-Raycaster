#include "RaycastRenderer.h"


RaycastRenderer::RaycastRenderer(int renderWidth, int renderHeight, Map* map, Player* player)
	: m_RenderWidth{ renderWidth }, m_RenderHeight{ renderHeight }, m_pEngineMap{ map }, m_pEnginePlayer{ player }, m_FOV{ FIELD_OF_VIEW }
{
	m_ProjectionPlaneCenter = renderHeight / 2;
	m_DistanceToProjectionPlane = (m_RenderWidth / 2) / abs(tan(util::radians(m_FOV / 2)));
	m_ScreenHandler = new ScreenHandler(m_RenderWidth, m_RenderHeight);
}

RaycastRenderer::~RaycastRenderer()
{
	delete m_ScreenHandler;
}

bool RaycastRenderer::init()
{
	if (!m_ScreenHandler->init())
	{
		std::cout << "Unable to initialize the screen handler\n";
		return false;
	}

	return true;
}

uint32_t* RaycastRenderer::getScreenPixels() { return m_ScreenHandler->getPointerToScreen(); }

pointf_t* RaycastRenderer::getIntersections() { return m_Intersections; }

void RaycastRenderer::paintScreen(uint32_t c)
{
	m_ScreenHandler->paintScreen(c);
}

void RaycastRenderer::raycast()
{
	// Send a ray out into the scene for each vertical row of pixels in the screen array
	for (int x{ 0 }; x < m_RenderWidth; x++)
	{
		// Calculate the angle between two rays
		double angleBetween{ util::degrees(atan(static_cast<double>(x - (m_RenderWidth / 2)) / m_DistanceToProjectionPlane)) };

		// Find the angle of the ray
		double rayAngle{ m_pEnginePlayer->getA() - angleBetween };

		// Precalculate some values that will be used in the floor and ceiling casting loops below
		double cosOfThetaMinusRayAngle{ cos(util::radians(m_pEnginePlayer->getA() - rayAngle)) };

		// Precalculate a value that will be used more times
		double tanOfRayAngle{ tan(util::radians(rayAngle)) };

		// Find the angle of the ray on the interval 0 <= rayAngle < 360
		rayAngle = util::getCoterminalAngle(rayAngle);

		if (rayAngle == 360.0f)
			rayAngle = 0.0f;

		// These two boolean values are used to determine how to texture the wall by determining which side of the wall the ray hit
		// Remember, the compass is oriented so that north is at 90 degrees 
		side_t northOrSouth{};	// North means the ray will hit the top of the wall, south means it will hit the bottom
		side_t eastOrWest{};	// East means the ray will hit the left of the wall, west means it will hit the right

		// CALCULATE HORIZONTAL INTERSECTIONS

		// Point A is the point of the first intersection between the ray and the horizontal grid lines
		double aX{};
		double aY{};

		// The change between point A and the next intersection with the horizontal grid lines
		double adX{};
		double adY{};

		// If the ray is facing up... (or downwards on the coordinate grid)
		if (rayAngle < 180)
		{
			// Because the ray is pointing up, that means it will hit the bottoms of the wall
			northOrSouth = side_t::SOUTH;

			// The first intersection will be part of the grid below (calculates y-coordinate of grid line below)
			aY = floor(m_pEnginePlayer->getY() / static_cast<double>(m_pEngineMap->getCellSize())) * m_pEngineMap->getCellSize();

			// The next intersection with a horizontal grid line will be gridSize units below
			adY = -static_cast<double>(m_pEngineMap->getCellSize());

			//	      90					90		
			//  -x,-y | +x,-y			-tan | +tan
			// 180 ---+--- 0		  180 ---+--- 0	
			//	-x,+y |	+x,+y			+tan | -tan
			//		 270				    270		
			// When rayAngle < 90, dx should be >0, and when rayAngle > 90, dx should be <0
			// It just so happens that tan is >0 when rayAngle < 90 degrees, and tan is <0 when rayAngle > 90
			// so I don't have to change the signs at all
			adX = m_pEngineMap->getCellSize() / tanOfRayAngle;

			// Calculate the x-coordinate of the first intersection with a horizontal gridline
			aX = m_pEnginePlayer->getX() - (aY - m_pEnginePlayer->getY()) / tanOfRayAngle;

			// Make part of the grid below for ease of checking for a wall
			aY -= 0.001;
		}
		// If ray is facing down... (or upwards on the coordinate grid)
		else
		{
			// The ray is facing down, so the ray hits the top of the wall
			northOrSouth = side_t::NORTH;

			// The first horizontal grid intersection is with the grid line above the m_pEnginePlayer
			aY = floor(m_pEnginePlayer->getY() / static_cast<double>(m_pEngineMap->getCellSize())) * m_pEngineMap->getCellSize() + m_pEngineMap->getCellSize();

			// The next gridline with be gridSize units above the m_pEnginePlayer
			adY = static_cast<double>(m_pEngineMap->getCellSize());

			//	      90					90		
			//  -x,-y | +x,-y			-tan | +tan
			// 180 ---+--- 0		  180 ---+--- 0	
			//	-x,+y |	+x,+y			+tan | -tan
			//		 270				    270		
			// When rayAngle < 270, dx should be <0, and when rayAngle > 270, dx should be >0
			// It just so happens that tan is >0 when rayAngle < 270 degrees, and tan is <0 when rayAngle > 270
			// so I have to flip the signs with the negative
			adX = -m_pEngineMap->getCellSize() / tanOfRayAngle;

			// Calculate the x-coordinate of the first intersection with a horizontal gridline
			aX = m_pEnginePlayer->getX() - (aY - m_pEnginePlayer->getY()) / tanOfRayAngle;
		}

		// CALCULATE VERTICAL INTERSECTIONS (very similar to calculating horizontal intersections)

		// Point B is the point of the first intersection between the ray and the vertical grid lines
		double bX{};
		double bY{};

		// The change between point B and the next intersection with the vertical grid lines
		double bdX{};
		double bdY{};

		// If the ray is facing to the right...
		if (rayAngle < 90.0f || rayAngle > 270.0f)
		{
			// The ray is facing to the right, so it will hit the left wall
			eastOrWest = side_t::WEST;

			// The first intersection will be in a grid to the right of the current grid
			bX = floor(m_pEnginePlayer->getX() / m_pEngineMap->getCellSize()) * m_pEngineMap->getCellSize() + m_pEngineMap->getCellSize();

			// The ray is moving in a positive x-direction
			bdX = static_cast<double>(m_pEngineMap->getCellSize());

			//	      90					90		
			//  -x,-y | +x,-y			-tan | +tan
			// 180 ---+--- 0		  180 ---+--- 0	
			//	-x,+y |	+x,+y			+tan | -tan
			//		 270				    270		
			// When rayAngle < 180, dy should be <0, and when rayAngle > 180, dy should be >0
			// It just so happens that tan is >0 when rayAngle < 180 degrees, and tan is <0 when rayAngle > 180
			// so I have to flip the signs with the negative
			bdY = -tanOfRayAngle * m_pEngineMap->getCellSize();

			// Calculate the y-coordinate of the first intersection with a vertical gridline
			bY = m_pEnginePlayer->getY() + (m_pEnginePlayer->getX() - bX) * tanOfRayAngle;
		}
		// If the ray is facing to the left...
		else
		{
			// The ray is facing left so it will hit the wall to the right
			eastOrWest = side_t::EAST;

			// The first intersection will be in a grid to the left
			bX = floor(m_pEnginePlayer->getX() / m_pEngineMap->getCellSize()) * m_pEngineMap->getCellSize();

			// The ray is moving in a negative x-direction
			bdX = -static_cast<double>(m_pEngineMap->getCellSize());

			//	      90					90		
			//  -x,-y | +x,-y			-tan | +tan
			// 180 ---+--- 0		  180 ---+--- 0	
			//	-x,+y |	+x,+y			+tan | -tan
			//		 270				    270		
			// When rayAngle < 180, dy should be <0, and when rayAngle > 180, dy should be >0
			// It just so happens that tan is <0 when rayAngle < 180 degrees, and tan is >0 when rayAngle > 180
			// so I don't have to change the signs at all
			bdY = tanOfRayAngle * m_pEngineMap->getCellSize();

			// Calculate the y-coordinate of the first intersection with a vertical gridline
			bY = m_pEnginePlayer->getY() + (m_pEnginePlayer->getX() - bX) * tanOfRayAngle;

			bX -= 0.001;
		}

		// CAST THE RAY
		
		// Distance to intersections with horizontal grid lines
		double horizontalDistance{ -1.0 };

		// Distance to intersections with vertical grid lines
		double verticalDistance{ -1.0 };

		// Distance from the m_pEnginePlayer to the point where the ray intersects the wall
		double distance{ -1.0 };

		// Cell coordinates of the intersection
		double xGrid{};
		double yGrid{};

		// The intersection point
		double iX{};
		double iY{};

		// The side of the cell that the ray intersects (north, south, east or west)
		side_t sideHit{};

		// Move the ray forward until it intersects with a wall
		do
		{
			// Calculate the not-square rooted distance from the m_pEnginePlayer to the next intersection points
			horizontalDistance = (m_pEnginePlayer->getX() - aX) * (m_pEnginePlayer->getX() - aX) + (m_pEnginePlayer->getY() - aY) * (m_pEnginePlayer->getY() - aY);
			verticalDistance = (m_pEnginePlayer->getX() - bX) * (m_pEnginePlayer->getX() - bX) + (m_pEnginePlayer->getY() - bY) * (m_pEnginePlayer->getY() - bY);

			double notSquaredDist{};

			// If a horizontal intersection is closest, than consider that intersection to see if the ray hit a wall
			if (horizontalDistance < verticalDistance)
			{
				notSquaredDist = horizontalDistance;
				iX = aX;
				iY = aY;

				// Move to the next horizontal intersection
				aX += adX;
				aY += adY;

				sideHit = northOrSouth;
			}
			// Otherwise, use the vertical intersection
			else
			{
				notSquaredDist = verticalDistance;
				iX = bX;
				iY = bY;

				// Move to the next vertical intersection
				bX += bdX;
				bY += bdY;

				sideHit = eastOrWest;
			}

			// Calculate the cell coordinates of the intersection
			xGrid = static_cast<int>(iX / m_pEngineMap->getCellSize());
			yGrid = static_cast<int>(iY / m_pEngineMap->getCellSize());

			// Test if there is a wall at those cell coordinates
			if (!(xGrid < 0 || xGrid >= m_pEngineMap->getWidth() || yGrid < 0 || yGrid >= m_pEngineMap->getHeight()))
			{
				if (m_pEngineMap->get(xGrid, yGrid).wall)
				{
					distance = sqrt(notSquaredDist);
				}
			}
		} while (distance < 0.0);

		m_Intersections[x].x = iX;
		m_Intersections[x].y = iY;

		// A number on the interval 0 < cellSpaceColumn < cellSize. The column on the wall where the ray intersects the wall
		int cellSpaceColumn{};
		Texture* texture{};

		switch (sideHit)
		{
		case side_t::NORTH:
		{
			texture = m_pEngineMap->get(xGrid, yGrid).textures[0];

			int topLeftX{ static_cast<int>(iX / m_pEngineMap->getCellSize()) * m_pEngineMap->getCellSize() + (m_pEngineMap->getCellSize() - 1) };
			cellSpaceColumn = topLeftX - static_cast<int>(iX);
		}
			break;
		case side_t::EAST:
		{
			texture = m_pEngineMap->get(xGrid, yGrid).textures[1];

			int topLeftY{ static_cast<int>(iY / m_pEngineMap->getCellSize()) * m_pEngineMap->getCellSize() };
			cellSpaceColumn = static_cast<int>(iY) - topLeftY;
		}
			break;
		case side_t::SOUTH:
		{
			texture = m_pEngineMap->get(xGrid, yGrid).textures[2];

			int bottomRightX{ static_cast<int>(iX / m_pEngineMap->getCellSize()) * m_pEngineMap->getCellSize() };
			cellSpaceColumn = static_cast<int>(iX) - bottomRightX;
		}
			break;
		case side_t::WEST:
		{
			texture = m_pEngineMap->get(xGrid, yGrid).textures[3];

			int bottomRightY{ static_cast<int>(iY / m_pEngineMap->getCellSize()) * m_pEngineMap->getCellSize() + (m_pEngineMap->getCellSize() - 1) };
			cellSpaceColumn = bottomRightY - static_cast<int>(iY);
		}
			break;
		}

		double correctedDistance{ distance * cosOfThetaMinusRayAngle };

		// Calculate the lighting each wall sliver experiences, if the m_pEnginePlayer were a light
		double lighting = -0.6 * distance + 255;

		// If the light level is less than 0, clamp to zero
		if (lighting < 0)
			lighting = 0;

		renderWallSlice(x, correctedDistance, cellSpaceColumn, lighting, texture);
	}
}

void RaycastRenderer::renderWallSlice(int x, double distance, int cellSpaceColumn, double light, Texture* texture)
{
	int wallHeight{ static_cast<int>(ceil((m_DistanceToProjectionPlane * m_pEngineMap->getMaxHeight()) / distance)) };

	// Calculate the top and the bottom of the wall
	int bottomOfWall{ static_cast<int>(ceil(m_ProjectionPlaneCenter + (m_DistanceToProjectionPlane * m_pEnginePlayer->getZ()) / distance)) };
	int topOfWall{ bottomOfWall - wallHeight };

	if (bottomOfWall > m_MaxSliceY)
		m_MaxSliceY = bottomOfWall;

	if (topOfWall < m_MinSliceY)
		m_MinSliceY = topOfWall;

	int textureSpaceColumn{ static_cast<int>(static_cast<float>(cellSpaceColumn) / m_pEngineMap->getCellSize() * texture->width()) };

	int minBetweenHeightAndStartY{ std::min(m_RenderHeight, bottomOfWall) };

	double wallHeightTextureHeightRatio{ static_cast<double>(texture->height()) / wallHeight };

	// Draw the wall sliver
	for (int y{ std::max(topOfWall, 0) }; y < minBetweenHeightAndStartY; y++)
	{
		
		int textureSpaceRow{ static_cast<int>((y - topOfWall) * wallHeightTextureHeightRatio) };

		m_ScreenHandler->setPixel(x, y, texture->getTexel(textureSpaceColumn, textureSpaceRow));
	}
}

void RaycastRenderer::renderFullFloor()
{
	// Precalculate some values that will be used in the loop below
	double cosOfPlayerAMinusHalfFOV{ cos(util::radians(m_pEnginePlayer->getA() - m_FOV * 0.5)) };
	double sinOfPlayerAMinusHalfFOV{ sin(util::radians(m_pEnginePlayer->getA() - m_FOV * 0.5)) };

	double cosOfPlayerAPlusHalfFOV{ cos(util::radians(m_pEnginePlayer->getA() + m_FOV * 0.5)) };
	double sinOfPlayerAPlusHalfFOV{ sin(util::radians(m_pEnginePlayer->getA() + m_FOV * 0.5)) };

	double cosOfHalfFOV{ cos(util::radians(m_FOV * 0.5)) };

	// Draw Floor
	// Loop over every horizontal line from the center of the projection plane down. Exclude the line at the center because
	// it would cause a divide by 0 error
	for (int y{ std::max(m_ProjectionPlaneCenter + 1, 0) }; y < m_RenderHeight; y++)
	{
		// Calculate how far the strip of floor corresponding to the horizontal line on the projection plane is from the player
		double distanceToStripe{ (m_DistanceToProjectionPlane * static_cast<double>(m_pEnginePlayer->getZ())) / (y - m_ProjectionPlaneCenter) };

		// Calculate the corrected distance (the walls are calculated with the corrected distance, so the floor must be as well)
		double correctedDistance{ distanceToStripe / cosOfHalfFOV };

		// Calculate the point on the horizontal stripe to the far left of the FOV
		double adX{ correctedDistance * cosOfPlayerAMinusHalfFOV };
		double adY{ correctedDistance * -sinOfPlayerAMinusHalfFOV };

		double aX{ m_pEnginePlayer->getX() + adX };
		double aY{ m_pEnginePlayer->getY() + adY };

		// Calculate the point on the horizontal stripe to the far right of the FOV
		double bdX{ correctedDistance * cosOfPlayerAPlusHalfFOV };
		double bdY{ correctedDistance * -sinOfPlayerAPlusHalfFOV };

		double bX{ m_pEnginePlayer->getX() + bdX };
		double bY{ m_pEnginePlayer->getY() + bdY };

		// Calculate a vector that points from point A to point B
		double vX{ bX - aX };
		double vY{ bY - aY };

		// Divide the vector by the width of the screen so that each time the vector is added to point A, a new pixel 
		// on the screen is represented
		double floorStepX{ vX / m_RenderWidth };
		double floorStepY{ vY / m_RenderWidth };

		// A for loop that goes accross the horizontal line
		for (int x{ m_RenderWidth - 1 }; x >= 0; x--)
		{
			// If the point where the floor is to be sampled is outside of the map, move to the next point
			if (aX < 0.0 || aX > (m_pEngineMap->getWidth() - 1) * m_pEngineMap->getCellSize() || aY < 0.0 || aY > (m_pEngineMap->getHeight() - 1) * m_pEngineMap->getCellSize())
			{
				aX += floorStepX;
				aY += floorStepY;
				continue;
			}

			// Calculate the coordinates of the grid square the point on the floor is in
			int gridX{ static_cast<int>(abs(aX) / m_pEngineMap->getCellSize()) * m_pEngineMap->getCellSize() };
			int gridY{ static_cast<int>(abs(aY) / m_pEngineMap->getCellSize()) * m_pEngineMap->getCellSize() };

			// Calculate the normalized coordinates of the point in terms of the grid square it is in
			double normX{ (abs(aX) - gridX) / static_cast<double>(m_pEngineMap->getCellSize()) };
			double normY{ (abs(aY) - gridY) / static_cast<double>(m_pEngineMap->getCellSize()) };

			if (aX < 0.0)
				normX = 1.0 - normX;

			if (aY < 0.0)
				normY = 1.0 - normY;

			// Write the color to the screen array
			m_ScreenHandler->setPixel(x, y, m_pEngineMap->get(gridX / m_pEngineMap->getCellSize(), gridY / m_pEngineMap->getCellSize()).floor->getTexel(normX, normY));

			// Increment point A to get the next point on the floor
			aX += floorStepX;
			aY += floorStepY;
		}
	}
}

void RaycastRenderer::renderFullCeiling()
{
	// Precalculate some values that will be used in the loop below
	double cosOfPlayerAMinusHalfFOV{ cos(util::radians(m_pEnginePlayer->getA() - m_FOV * 0.5)) };
	double sinOfPlayerAMinusHalfFOV{ sin(util::radians(m_pEnginePlayer->getA() - m_FOV * 0.5)) };

	double cosOfPlayerAPlusHalfFOV{ cos(util::radians(m_pEnginePlayer->getA() + m_FOV * 0.5)) };
	double sinOfPlayerAPlusHalfFOV{ sin(util::radians(m_pEnginePlayer->getA() + m_FOV * 0.5)) };

	double cosOfHalfFOV{ cos(util::radians(m_FOV * 0.5)) };

	// Draw Ceiling
	// Loop over every horizontal line from the center of the projection plane up. Exclude the line at the center because
	// it would cause a divide by 0 error
	for (int y{ std::min(m_ProjectionPlaneCenter - 1, m_RenderHeight - 1) }; y >= 0; y--)
	{
		// Calculate how far the strip of ceiling corresponding to the horizontal line on the projection plane is from the player
		double distanceToStripe{ (m_DistanceToProjectionPlane * static_cast<double>(m_pEngineMap->getMaxHeight() - m_pEnginePlayer->getZ())) / (m_ProjectionPlaneCenter - y) };

		// Calculate the corrected distance (the walls are drawn with the corrected distance, so the ceiling must be as well)
		double correctedDistance{ distanceToStripe / cosOfHalfFOV };

		// Calculate the point on the horizontal stripe to the far left of the FOV
		double adX{ correctedDistance * cosOfPlayerAMinusHalfFOV };
		double adY{ correctedDistance * -sinOfPlayerAMinusHalfFOV };

		double aX{ m_pEnginePlayer->getX() + adX };
		double aY{ m_pEnginePlayer->getY() + adY };

		// Calculate the point on the horizontal stripe to the far right of the FOV
		double bdX{ correctedDistance * cosOfPlayerAPlusHalfFOV };
		double bdY{ correctedDistance * -sinOfPlayerAPlusHalfFOV };

		double bX{ m_pEnginePlayer->getX() + bdX };
		double bY{ m_pEnginePlayer->getY() + bdY };

		// Calculate a vector that points from point A to point B
		double vX{ bX - aX };
		double vY{ bY - aY };

		// Divide the vector by the width of the screen so that each time the vector is added to point A, a new pixel 
		// on the screen is represented
		double floorStepX{ vX / m_RenderWidth };
		double floorStepY{ vY / m_RenderWidth };

		// A for loop that goes accross the horizontal line
		for (int x{ m_RenderWidth - 1 }; x >= 0; x--)
		{
			// If the point where the ceiling is to be sampled is outside of the map, move to the next point
			if (aX < 0.0 || aX >(m_pEngineMap->getWidth() - 1) * m_pEngineMap->getCellSize() || aY < 0.0 || aY >(m_pEngineMap->getHeight() - 1) * m_pEngineMap->getCellSize())
			{
				aX += floorStepX;
				aY += floorStepY;
				continue;
			}

			// Calculate the coordinates of the grid square the point on the ceiling is in
			int gridX{ static_cast<int>(abs(aX) / m_pEngineMap->getCellSize()) * m_pEngineMap->getCellSize() };
			int gridY{ static_cast<int>(abs(aY) / m_pEngineMap->getCellSize()) * m_pEngineMap->getCellSize() };

			// Calculate the normalized coordinates of the point in terms of the grid square it is in
			double normX{ (abs(aX) - gridX) / static_cast<double>(m_pEngineMap->getCellSize()) };
			double normY{ (abs(aY) - gridY) / static_cast<double>(m_pEngineMap->getCellSize()) };

			if (aX < 0.0)
				normX = 1.0 - normX;

			if (aY < 0.0)
				normY = 1.0 - normY;

			// Write the color to the screen array
			m_ScreenHandler->setPixel(x, y, m_pEngineMap->get(gridX / m_pEngineMap->getCellSize(), gridY / m_pEngineMap->getCellSize()).ceiling->getTexel(normX, normY));

			// Increment point A to get the next point on the ceiling
			aX += floorStepX;
			aY += floorStepY;
		}
	}
}
/*
* Created 2:41 PM on 12/25/2021 (Merry Christmas / Clashmas)
*/

#include <iostream>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include <vector>
#include <algorithm>

// Headers created by me which contain useful classes
#include "Texture.h"
#include "Sprites.h"

// Size of the screen which the raycast scene is projected to (doesn't include the map)
const int width = 640;
const int height = 400;

int projectionPlaneCenter{ height / 2 };	// The vertical center of the projection plane

bool DEBUG{ false };	// Set equal to true for an overhead view of the scene

// Stores the distance to the scene at each column
float zBuffer[width];

const int gridSize{ 64 };		// Side length of an individual grid block
const int gridWidth{ 20 };		// Width of the whole map in terms of grid blocks
const int gridHeight{ 20 };		// Height of the whole map in terms of grid blocks
std::string gridMap{};	// String which stores the map
std::vector<int> wallHeights(gridWidth * gridHeight);

Texture* wallTextureLocations[gridWidth * gridHeight];
Texture* floorTextureLocations[gridWidth * gridHeight];
Texture* ceilingTextureLocations[gridWidth * gridHeight];

// Vector which holds all the sprites
std::vector<Sprite> sprites{};

int FOV{ 60 };							// Field of view of player
int distanceToProjectionPlane{ 277 };	// Distance of the "camera" (player) to the "projection plane" (screen)

int playerHeight{ gridSize / 2 };		// Height of player (typically half of gridSize)
int playerRadius{ 15 };					// Radius of player

// The "adjusted" distance to the projection plane, used so that I can adjust the field of view
float adjustedDistanceToProjectionPlane{ (width / 2) / fabs(tanf((FOV / 2) * (M_PI / 180.0f))) };

float theta{ 0.0f };					// Angle of the player
float playerX{ 432.224f };				// x-coordinate of the player in pixels, not grid coordinates
float playerY{ 353.454f };				// y-coordinate of the player in pixels, not grid coordinates
float playerSpeed{ 100.0f };			// Speed at which the player moves
float playerTurnSpeed{ 75.0f };			// Speed with which the player can turn
float playerLookUpSpeed{ 175.0f };		// Speed with which the player can look up and down (modifies projectionPlaneCenter variable)

// Used to calculate the time elapsed between frames
float previousTime{};
float currentTime{};
float deltaTime{};
float FPS{};

// Struct for debugging (holds an intersection point)
struct point
{
	float x{};
	float y{};
};

std::vector<point> aPoints(width);		// Holds intersections with horizontal gridlines
std::vector<point> bPoints(width);		// Holds intersections with vertical gridlines
std::vector<point> actualPoints(width);	// Holds the intersection points that are used in rendering
std::vector<point> floorPoints{};		// Points where the floor texture is sampled

enum Side
{
	Top,
	Bottom,
	Left,
	Right
};

struct CastPoint
{
	float x{};
	float y{};
	float dist{};
	Side side{};
};

// Convert an angle from degrees to radians (needed for use with trigonometric functions)
float radians(float degrees)
{
	return static_cast<float>(degrees * (M_PI / 180.0f));
}

float degrees(float radians)
{
	return static_cast<float>(radians * (180.0f / M_PI));
}

// Restrict angles to 0 - 360 degrees
float getCoterminalAngle(float angle)
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
		return angle - (static_cast<int>(angle / 360) * 360);
	else return angle;
}

uint32_t calculateLighting(const uint32_t& color, const float& lighting)
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

CastPoint rayCast(const std::string& map, float x, float y, float theta)
{
	float tanOfRayAngle{ tanf(radians(theta)) };

	// Find the angle of the ray on the interval 0 <= theta < 360
	theta = getCoterminalAngle(theta);

	if (theta == 360.0f)
		theta = 0.0f;

	// These two boolean values are used to determine how to texture the wall by determining which side of the wall the ray hit
	bool topOrBottom{};	// True means the ray hit the top of the wall, false means it hit the bottom
	bool leftOrRight{};	// True means the ray hit the left of the wall, false means it hit the right

	// CALCULATE HORIZONTAL INTERSECTIONS
	float horizontalIntersectionsDistance{ -1.0f };

	// Point A is the point of the first intersection between the ray and the horizontal grid lines
	float aX{};
	float aY{};

	// The change between point A and the next intersection with the horizontal grid lines
	float dx{};
	float dy{};

	// If the ray is facing up... (or downwards on the coordinate grid)
	if (theta < 180)
	{
		// Because the ray is pointing up, that means it will hit the bottoms of the wall
		topOrBottom = false;

		// The first intersection will be part of the grid below (calculates y-coordinate of grid line below)
		aY = floorf(y / static_cast<float>(gridSize)) * gridSize;

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
		aX = x - (aY - y) / tanOfRayAngle;

		// Make part of the grid below for ease of checking for a wall
		aY--;
	}
	// If ray is facing down... (or upwards on the coordinate grid)
	else
	{
		// The ray is facing down, so the ray hits the top of the wall
		topOrBottom = true;

		// The first horizontal grid intersection is with the grid line above the player
		aY = floorf(y / static_cast<float>(gridSize)) * gridSize + gridSize;

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
		aX = x - (aY - y) / tanOfRayAngle;
	}

	// Grid coordinates of point A
	int aXgrid{ static_cast<int>(aX / gridSize) };
	int aYgrid{ static_cast<int>(aY / gridSize) };

	// So long as the x-coordinate in terms of the grid of A is within the bounds of the map...
	if (!(aXgrid < 0 || aXgrid >= gridWidth))
	{
		// If there is a wall in that grid, calculate the distance
		if (map[aYgrid * gridWidth + aXgrid] == '#')
		{
			horizontalIntersectionsDistance = sqrtf((x - aX) * (x - aX) + (y - aY) * (y - aY));
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
		if (!(aXgrid < 0 || aXgrid >= gridWidth))
		{
			if (map[aYgrid * gridWidth + aXgrid] == '#')
			{
				horizontalIntersectionsDistance = sqrtf((x - aX) * (x - aX) + (y - aY) * (y - aY));;
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
	if (theta < 90.0f || theta > 270.0f)
	{
		// The ray is facing to the right, so it will hit the left wall
		leftOrRight = true;

		// The first intersection will be in a grid to the right of the current grid
		bX = floorf(x / gridSize) * gridSize + gridSize;

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
		bY = y + (x - bX) * tanOfRayAngle;
	}
	// If the ray is facing to the left...
	else
	{
		// The ray is facing left so it will hit the wall to the right
		leftOrRight = false;

		// The first intersection will be in a grid to the left
		bX = floorf(x / gridSize) * gridSize;

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
		bY = y + (x - bX) * tanOfRayAngle;

		bX--;
	}

	// Same process as with the horizontal intersection code
	int bXgrid{ static_cast<int>(bX / gridSize) };
	int bYgrid{ static_cast<int>(bY / gridSize) };

	if (!(bYgrid < 0 || bYgrid >= gridHeight))
	{
		if (map[bYgrid * gridWidth + bXgrid] == '#')
		{
			verticalIntersectionsDistance = sqrtf((x - bX) * (x - bX) + (y - bY) * (y - bY));
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

		if (!(bYgrid < 0 || bYgrid >= gridHeight))
		{
			if (map[bYgrid * gridWidth + bXgrid] == '#')
			{
				verticalIntersectionsDistance = sqrtf((x - bX) * (x - bX) + (y - bY) * (y - bY));
			}
		}
		else
		{
			verticalIntersectionsDistance = FLT_MAX;
		}
	}

	// The column the ray hits on a wall
	int gridSpaceColumn{};

	CastPoint intersection{};

	// The ray used for rendering is the one closest to the wall, so save the one which is a smaller distance away to the actual intersection
	// points vector
	if (horizontalIntersectionsDistance < verticalIntersectionsDistance)
	{
		intersection.x = aX;
		intersection.y = aY;
		intersection.dist = horizontalIntersectionsDistance;

		// If the ray hit the top of a wall...
		if (topOrBottom)
		{
			intersection.side = Top;
		}
		// If the ray hit the bottom of a wall...
		else
		{
			intersection.side = Bottom;
		}
	}
	else
	{
		intersection.x = bX;
		intersection.y = bY;
		intersection.dist = verticalIntersectionsDistance;

		// If the ray hit the left side of a wall...
		if (leftOrRight)
		{
			intersection.side = Left;
		}
		// If the ray hit the right side of a wall...
		else
		{
			intersection.side = Right;
		}
	}

	return intersection;
}

int getGridX(const CastPoint& intersection)
{
	int x{ static_cast<int>(intersection.x) };
	int y{ static_cast<int>(intersection.y) };

	int gridSpaceColumn{};
	switch (intersection.side)
	{
		// If the ray hit the top of a wall...
	case Top:
	{
		// First column on the top of the wall; at the top left corner of the wall
		int gridX{ static_cast<int>(x / gridSize) * gridSize + (gridSize - 1) };
		gridSpaceColumn = gridX - x;
	}
		break;
		// If the ray hit the right side of a wall...
	case Right:
	{
		// First column on the right side of the wall; at the bottom right corner of the wall
		int gridY{ static_cast<int>(y / gridSize) * gridSize + (gridSize - 1) };
		gridSpaceColumn = gridY - y;
	}
		break;
		// If the ray hit the bottom of a wall...
	case Bottom:
	{
		// First column on the bottom of the wall; at the bottom right corner of the wall
		int gridX{ static_cast<int>(x / gridSize) * gridSize };
		gridSpaceColumn = x - gridX;
	}
		break;
		// If the ray hit the left side of a wall...
	case Left:
	{
		// First column on the left side of the wall; at the top left corner of the wall
		int gridY{ static_cast<int>(y / gridSize) * gridSize };
		gridSpaceColumn = y - gridY;
	}
		break;
	}

	return gridSpaceColumn;
}



int main(int argc, char* argv[])
{
	// SDL_Init() returns a negative number upon failure, and SDL_INIT_EVERYTHING sets all the flags to true
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		std::cout << "Error initializing SDL: " << SDL_GetError() << '\n';	// SDL_GetError() returns information about an error that occurred in a string

	// VVVVVVVVVVVVVVVVVVVVVVV Put image flags here
	int imgFlags{ IMG_INIT_PNG };
	if (!(IMG_Init(imgFlags) & imgFlags))
		std::cout << "Error initializing IMG: " << IMG_GetError() << '\n';

	// Initialize TTF
	if (TTF_Init() < 0)
		std::cout << "Error initializing TTF: " << TTF_GetError() << '\n';

	// Open the audio
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
		std::cout << "Error opening Mix: " << Mix_GetError() << '\n';

	// SDL_CreateWindow() creates a window
	//								Window name	  Window X position     Window Y position   width height    flags
	//									 V			      V						V			   V    V         V
	SDL_Window* win{ SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (DEBUG ? width + height : width), height, 0) };
	SDL_Renderer* renderTarget{ SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED) };

	bool isRunning{ true };
	SDL_Event ev{};

	// Create a blank texture
	SDL_Texture* frameBuffer{ SDL_CreateTexture(renderTarget, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height) };
	uint32_t* screen = new uint32_t[width * height];	// An array of pixels that is manipulated then updated to frameBuffer

	const Uint8* keystate{};

	// Textures for texturing the ceiling, floor, walls, and sprites
	Texture noTexture{ "redbrick.png", SDL_PIXELFORMAT_RGBA8888 };
	Texture redbrick{ "redbrick.png", SDL_PIXELFORMAT_RGBA8888 };
	Texture colorstone{ "colorstone.png", SDL_PIXELFORMAT_RGBA8888 };
	Texture wood{ "wood.png", SDL_PIXELFORMAT_RGBA8888 };
	Texture purplestone{ "purplestone.png", SDL_PIXELFORMAT_RGBA8888 };
	Texture bluestone{ "bluestone.png", SDL_PIXELFORMAT_RGBA8888 };
	Texture greystone{ "greystone.png", SDL_PIXELFORMAT_RGBA8888 };
	Texture mossy{ "mossy.png", SDL_PIXELFORMAT_RGBA8888 };
	Texture eagle{ "eagle.png", SDL_PIXELFORMAT_RGBA8888 };

	Texture front{ "sprite-sheet-non-transparent.png", {32, 0, 32, 32}, SDL_PIXELFORMAT_RGBA8888 };
	Texture left{ "sprite-sheet-non-transparent.png", {32, 32, 32, 32}, SDL_PIXELFORMAT_RGBA8888 };
	Texture right{ "sprite-sheet-non-transparent.png", {32, 64, 32, 32}, SDL_PIXELFORMAT_RGBA8888 };
	Texture back{ "sprite-sheet-non-transparent.png", {32, 96, 32, 32}, SDL_PIXELFORMAT_RGBA8888 };

	Texture spriteTexture{ "greenlight.png", SDL_PIXELFORMAT_RGBA8888 };
	Texture spriteTexture2{ "pillar.png", SDL_PIXELFORMAT_RGBA8888 };

	// Add sprites to the sprite array
	/*sprites.push_back(Sprite{ &spriteTexture, gridSize * 5.5f, gridSize * 17.5f });
	sprites.push_back(Sprite{ &spriteTexture, 350.0f + 127.0f, 350.0f });
	sprites.push_back(Sprite{ &spriteTexture2, 11.5f * gridSize, 13.5f * gridSize });
	sprites.push_back(Sprite{ &spriteTexture2, 18.5f * gridSize, 13.5f * gridSize });
	sprites.push_back(Sprite{ {&front, &right, &back, &left}, 16.0f * gridSize, 5.0f * gridSize, 0.0f });*/

	// Create the map
	gridMap += "####################";
	gridMap += "#--------##--------#";
	gridMap += "#####-#####--------#";
	gridMap += "#------------------#";
	gridMap += "#------------------#";
	gridMap += "#-##-----##--------#";
	gridMap += "#-##-----##--------#";
	gridMap += "#--------##--------#";
	gridMap += "#--####--##--------#";
	gridMap += "#-########--########";
	gridMap += "#-######---#########";
	gridMap += "#---#----##--------#";
	gridMap += "#-#---#########-####";
	gridMap += "#####-----#--------#";
	gridMap += "#-----#------------#";
	gridMap += "##-#-##--##-##---#-#";
	gridMap += "#--#-###-##-##-----#";
	gridMap += "#-##--#--##--------#";
	gridMap += "#--####--##--####--#";
	gridMap += "####################";

	for (int x{ 0 }; x < gridWidth; x++)
	{
		for (int y{ 0 }; y < gridHeight; y++)
		{
			if (gridMap[y * gridWidth + x] == '#')
			{
				wallHeights[y * gridWidth + x] = gridSize;
			}
			else if (gridMap[y * gridWidth + x] == ' -' )
			{
				wallHeights[y * gridWidth + x] = 0;
			}
		}
	}

	wallHeights[5 * gridWidth + 15] = gridSize / 2;
	wallHeights[5 * gridWidth + 9] = gridSize / 2;

	for (int y{ 0 }; y < gridHeight; y++)
	{
		for (int x{ 0 }; x < gridWidth; x++)
		{
			int location{ y * gridWidth + x };
			if (gridMap[location] == '#')
			{
				floorTextureLocations[location] = &noTexture;
				ceilingTextureLocations[location] = &noTexture;

				if (x < gridWidth / 2 && y < gridHeight / 2)
					wallTextureLocations[location] = &redbrick;
				else if (x > gridWidth / 2 && y < gridHeight / 2)
					wallTextureLocations[location] = &purplestone;
				else if (x < gridWidth / 2 && y < gridHeight)
					wallTextureLocations[location] = &bluestone;
				else
					wallTextureLocations[location] = &eagle;
			}
			else
			{
				wallTextureLocations[location] = &noTexture;

				ceilingTextureLocations[location] = &mossy;
				floorTextureLocations[location] = &wood;
			}
		}
	}

	// Game loop
	while (isRunning)
	{
		// Event loop
		while (SDL_PollEvent(&ev) != 0)
		{
			switch (ev.type)
			{
				// Check if the exit button has been clicked
			case SDL_QUIT:
				isRunning = false;
				break;

				// Check for window events
			case SDL_WINDOWEVENT:
				// If the window is minimized, wait for events. Fixes memory spikes
				if (ev.window.event == SDL_WINDOWEVENT_MINIMIZED)
				{
					while (SDL_WaitEvent(&ev))
					{
						if (ev.window.event == SDL_WINDOWEVENT_RESTORED)
							break;
					}
				}
				break;
			}
		}

		// Get keys
		keystate = SDL_GetKeyboardState(NULL);

		// Calculate deltaTime and the frames per second
		previousTime = currentTime;
		currentTime = SDL_GetTicks() / 1000.0f;
		deltaTime = currentTime - previousTime;
		FPS = 1.0f / deltaTime;

		// Make a copy of the map so I can put in a character to represent the player without changing the original map
		std::string mapCopy{ gridMap };

		// Calculate player coordinates in terms of grid squares
		int gridX{ static_cast<int>(playerX / gridSize) };
		int gridY{ static_cast<int>(playerY / gridSize) };

		// Put a character to represent the player in the map
		mapCopy[gridY * gridWidth + gridX] = 'P';

		// Output FPS and angle info
		std::cout << "FPS: " << FPS << '\n';
		std::cout << "X: " << playerX << ", Y: " << playerY << ", Angle: " << theta << '\n';

		// Copy the map to the console
		for (int y{ 0 }; y < gridHeight; y++)
		{
			for (int x{ 0 }; x < gridWidth; x++)
			{
				std::cout << mapCopy[y * gridWidth + x];
			}
			std::cout << '\n';
		}

		// Return the cursor in the console to twelve lines above so that old information is written over
		std::cout << "\x1b[" << 2 + gridHeight << "F";

		// Color every pixel in the screen array black
		for (int i{ 0 }; i < width * height; i++)
			screen[i] = 0xFF000000;

		// Calculate the seperate speed components of the player
		float xSpeed{};
		float ySpeed{};

		// ySpeed is negative because we're using the typical degree layout (unit circle), but y is increasing downward in out coordinate grid
		//	     90				 ----------> x
		//       |				|
		//180 ---+--- 0			|
		//		 |				v
		//		270				y
		// Because sine is positive in the first two quadrants on the unit circle, but that direction is down in our coordinate grid,
		// we need to make it negative to flip the sign

		// Move player forwards
		if (keystate[SDL_SCANCODE_W])
		{
			xSpeed = playerSpeed * cosf(radians(theta));
			ySpeed = -playerSpeed * sinf(radians(theta));
		}

		// Move player backwards
		if (keystate[SDL_SCANCODE_S])
		{
			xSpeed = -playerSpeed * cosf(radians(theta));
			ySpeed = playerSpeed * sinf(radians(theta));
		}

		playerX += xSpeed * deltaTime;
		playerY += ySpeed * deltaTime;

		// Turn player left and right
		if (keystate[SDL_SCANCODE_A])
			theta += playerTurnSpeed * deltaTime;
		else if (keystate[SDL_SCANCODE_D])
			theta -= playerTurnSpeed * deltaTime;

		// Move player view up and down
		if (keystate[SDL_SCANCODE_DOWN])
			projectionPlaneCenter -= playerLookUpSpeed * deltaTime;
		else if (keystate[SDL_SCANCODE_UP])
			projectionPlaneCenter += playerLookUpSpeed * deltaTime;

		// Make sure some part of the projection plane is always over the middle of the screen
		if (projectionPlaneCenter <= 0)
			projectionPlaneCenter = 0;
		else if (projectionPlaneCenter >= height)
			projectionPlaneCenter = height;

		// Move player up and down
		if (keystate[SDL_SCANCODE_SPACE])
			playerHeight++;
		else if (keystate[SDL_SCANCODE_LSHIFT])
			playerHeight--;

		// Make sure the player doesn't fly above or below the world
		if (playerHeight >= gridSize)
			playerHeight = gridSize - 1;
		else if (playerHeight <= 0)
			playerHeight = 1;


		// Collision detection
		int playerGridOffsetX{ static_cast<int>(playerX) - (gridX * gridSize) };
		int playerGridOffsetY{ static_cast<int>(playerY) - (gridY * gridSize) };

		if (xSpeed < 0.0f)
		{
			if (gridMap[gridY * gridWidth + (gridX - 1)] == '#' && playerGridOffsetX < playerRadius)
				playerX -= xSpeed * deltaTime;
		}
		else
		{
			if (gridMap[gridY * gridWidth + (gridX + 1)] == '#' && gridSize - playerGridOffsetX < playerRadius)
				playerX -= xSpeed * deltaTime;
		}

		if (ySpeed < 0.0f)
		{
			if (gridMap[(gridY - 1) * gridWidth + gridX] == '#' && playerGridOffsetY < playerRadius)
				playerY -= ySpeed * deltaTime;
		}
		else
		{
			if (gridMap[(gridY + 1) * gridWidth + gridX] == '#' && gridSize - playerGridOffsetY < playerRadius)
				playerY -= ySpeed * deltaTime;
		}

		if (DEBUG)
			floorPoints.clear();

		// Send a ray out into the scene for each vertical row of pixels in the screen array
		for (int x{ 0 }; x < width; x++)
		{
			// Calculate the angle between two rays
			float angleBetween{ degrees(atanf(static_cast<float>(x - (width / 2)) / adjustedDistanceToProjectionPlane)) };

			// Find the angle of the ray
			float rayAngle{ theta - angleBetween };

			Texture* wallTexture{};
			Texture* floorTexture{};

			// Precalculate some values that will be used in the floor and ceiling casting loops below
			float cosOfRayAngle{ cosf(radians(rayAngle)) };
			float sinOfRayAngle{ sinf(radians(rayAngle)) };
			float cosOfThetaMinusRayAngle{ cosf(radians(theta - rayAngle)) };

			int previousTopOfWall{ height };
			int previousWallHeight{ 0 };
			int floorHeight{ 0 };

			float totalDistance{ 0.0f };

			CastPoint intersection{ rayCast(gridMap, playerX, playerY, rayAngle) };

			bool finished{ false };
			while (!finished)
			{
				totalDistance += intersection.dist;

				float distance{ totalDistance };

				int intersectionGridX{ static_cast<int>(intersection.x / gridSize) };
				int intersectionGridY{ static_cast<int>(intersection.y / gridSize) };
				wallTexture = wallTextureLocations[intersectionGridY * gridWidth + intersectionGridX];
				int variableHeight{ wallHeights[intersectionGridY * gridWidth + intersectionGridX] };

				// If the wall height of the current wall is less than that of the previous wall, that means 
				// it will be obstructed from view and therefore shouldn't be drawn
				if (variableHeight < previousWallHeight)
					continue;

				int gridSpaceColumn{ getGridX(intersection) };

				// Correct fish-eye distortion for the actual rendering of the walls
				distance *= cosf(radians(theta - rayAngle));

				// I use the fisheye corrected distance because the comparison in the sprite rendering loop uses the fisheye corrected distance
				// to the sprite
				// zBuffer[x] = distance;

				// Calculate the height of the wall
				int wallHeight{ static_cast<int>((distanceToProjectionPlane / distance) * variableHeight) };

				// Y-coordinates of the bottom and top of the wall. Calculated in terms of player height and projection plane center (using similar
				// triangles) so that when the player height changes, the location of the wall will as well
				//	int bottomOfWall{ static_cast<int>(projectionPlaneCenter + (distanceToProjectionPlane * playerHeight) / distance) };
				int bottomOfWall{ static_cast<int>(projectionPlaneCenter + (distanceToProjectionPlane * (playerHeight - previousWallHeight)) / distance) };
				int topOfWall{ static_cast<int>(projectionPlaneCenter + (distanceToProjectionPlane * playerHeight) / distance) - wallHeight };

				// Floor cast
				// y is a point on the projection plane from the bottom of the wall to the end of the screen


				for (int y{ std::max(0, bottomOfWall) }; y < previousTopOfWall; y++)
				{
					// The straight, vertical line distance to the point on the floor
					float straightDistance{ static_cast<float>((playerHeight - previousWallHeight) * distanceToProjectionPlane) / (y - projectionPlaneCenter) };

					// The corrected distance to the point on the floor (reverse fisheye)
					float correctedDistance{ straightDistance / cosOfThetaMinusRayAngle };

					// x and y components of a vector with a length of correctedDistance and angle of rayAngle
					float dx{ correctedDistance * cosOfRayAngle };
					float dy{ correctedDistance * -sinOfRayAngle };

					// Calculate the location on the floor of the map of the current point
					float pX{ playerX + dx };
					float pY{ playerY + dy };

					// Check if the point is outside the map. Happens when the player's height is very small
					if (pX < 0.0f || pX >= gridWidth * gridSize || pY < 0.0f || pY >= gridHeight * gridSize)
						continue;

					// Calculate the pixel coordinates of the grid square point P is in
					int gridPX{ static_cast<int>(pX / gridSize) * gridSize };
					int gridPY{ static_cast<int>(pY / gridSize) * gridSize };

					floorTexture = floorTextureLocations[(gridPY / gridSize) * gridWidth + (gridPX / gridSize)];

					// Find the coordinates of point P within the grid square and normalize them
					float normX{ (static_cast<int>(pX) - gridPX) / static_cast<float>(gridSize) };
					float normY{ (static_cast<int>(pY) - gridPY) / static_cast<float>(gridSize) };

					// Calculate the coordinates of point P in texture space
					int textureX{ static_cast<int>(normX * floorTexture->m_width) };
					int textureY{ static_cast<int>(normY * floorTexture->m_height) };

					int i{ textureY * floorTexture->m_width + textureX };

					if (i < 0)
						std::cout << "Column: " << x << ", Row: " << y << '\n';

					screen[y * width + x] = (*floorTexture)[textureY * floorTexture->m_width + textureX];
				}

				// The column on the texture which corresponds to the position of the ray intersection with the wall
				int textureSpaceColumn{ static_cast<int>(static_cast<float>(gridSpaceColumn) / gridSize * wallTexture->m_width) };

				// If I put std::min(bottomOfWall, height) into the for loop, it would evaluate every iteration, which is wasteful
				// because the value doesn't change
				int minBetweenHeightAndBottomOfWall{ std::min(bottomOfWall, height) };
				
				// Draw the wall sliver
				for (int y{ std::max(topOfWall, 0) }; y < minBetweenHeightAndBottomOfWall; y++)
				{
					// The row on the texture
					int textureSpaceRow{ static_cast<int>((y - topOfWall) / static_cast<float>(wallHeight) * wallTexture->m_height) };

					// Get the color of the texture at the point on the wall (x, y)
					uint32_t color{ (*wallTexture)[textureSpaceRow * wallTexture->m_width + textureSpaceColumn] };

					screen[y * width + x] = color;
				}

				// Update previousWallHeight now that its purpose has been fulfilled
				previousWallHeight = variableHeight;

				previousTopOfWall = topOfWall;

				// If the heigh of the wall is the max height, then there is no need to continue finding 
				// wall intersections because this wall obstructs them from view
				if (variableHeight == gridSize || topOfWall < 0)
					finished = true;
				else
					intersection = rayCast(gridMap, intersection.x, intersection.y, rayAngle);
			}

			Texture* ceilingTexture{};

			// Ceiling casting. Basically the same process as floorcasting, except from the top of the wall up
			for (int y{ previousTopOfWall }; y > 0; y--)
			{
				// The straight, vertical line distance to the point on the ceiling
				float straightDistance{ static_cast<float>((gridSize - playerHeight) * distanceToProjectionPlane) / (projectionPlaneCenter - y) };

				// The corrected distance to the point on the ceiling (Reverse fish eye)
				float correctedDistance{ straightDistance / cosOfThetaMinusRayAngle };

				// x and y components of a vector with a length of correctedDistance and angle of rayAngle
				float dx{ correctedDistance * cosOfRayAngle };
				float dy{ correctedDistance * -sinOfRayAngle };

				// Calculate the location on the ceiling of the map of the current point
				float pX{ playerX + dx };
				float pY{ playerY + dy };

				// Check if the point is outside of the map. Happens when the player's height is large
				if (pX < 0.0f || pX >= gridWidth * gridSize || pY < 0.0f || pY >= gridHeight * gridSize)
					continue;

				// Calculate the pixel coordinates of the grid square point P is in
				int gridPX{ static_cast<int>(pX / gridSize) * gridSize };
				int gridPY{ static_cast<int>(pY / gridSize) * gridSize };

				ceilingTexture = ceilingTextureLocations[(gridPY / gridSize) * gridWidth + (gridPX / gridSize)];

				// Find the coordinates of point P within the grid square and normalize them
				float normX{ (static_cast<int>(pX) - gridPX) / static_cast<float>(gridSize) };
				float normY{ (static_cast<int>(pY) - gridPY) / static_cast<float>(gridSize) };

				// Calculate the coordinates of point P in texture space
				int textureX{ static_cast<int>(normX * ceilingTexture->m_width) };
				int textureY{ static_cast<int>(normY * ceilingTexture->m_height) };

				// Calculate the lighting at that point on the ceiling
				float lighting{ -0.4f * correctedDistance + 255.0f };

				if (lighting < 0.0f)
					lighting = 0.0f;

				// screen[y * width + x] = calculateLighting(ceilingTexture[textureY * ceilingTexture.m_width + textureX], lighting);
				screen[y * width + x] = (*ceilingTexture)[textureY * ceilingTexture->m_width + textureX];
			}

		}
		
		for (int i{ 0 }; i < sprites.size(); i++)
			sprites[i].noSqrtDistance = (playerX - sprites[i].x) * (playerX - sprites[i].x) + (playerY - sprites[i].y) * (playerY - sprites[i].y);

		// Sort sprites by distance
		std::sort(sprites.begin(), sprites.end(), [](const Sprite& a, const Sprite& b)
			{
				return (a.noSqrtDistance > b.noSqrtDistance ? true : false);
			}
		);

		// Loop through each sprite
		for (int i{ 0 }; i < sprites.size(); i++)
		{
			// Calculate the angle along the horizontal axis
			float spriteAngle{ degrees(atan2f(sprites[i].y - playerY, sprites[i].x - playerX)) };

			// Add the angle of the player so that sprite angle is relative to the player's perspective
			spriteAngle += theta;

			// Find an equivalent angle for sprite angle on the interval 0-360
			spriteAngle = getCoterminalAngle(spriteAngle);

			// If the sprite is behind the player, it can be disregarded
			if (spriteAngle > 90.0f && spriteAngle < 270.0f)
				continue;
			
			// Use the angle to determine which column of the screen the center of the sprite is on
			// (this equation is the same as the equation used to calculate the angle between rays, except solved for x)
			int x{ static_cast<int>(adjustedDistanceToProjectionPlane * tanf(radians(spriteAngle)) + (width / 2)) };

			// Calculate distance and correct for the fish eye effect
			float distance{ sqrtf(sprites[i].noSqrtDistance) };
			distance *= cosf(radians(spriteAngle));

			// Calculate the size of the sprite (same process as calculating the height of a wall slice)
			int spriteSize{ static_cast<int>((distanceToProjectionPlane * gridSize) / distance) };
			int halfSpriteSize{ spriteSize / 2 };

			// Exact same thing as for the walls
			int bottomOfSprite{ projectionPlaneCenter + static_cast<int>((distanceToProjectionPlane * playerHeight) / distance) };
			int topOfSprite{ bottomOfSprite - spriteSize };

			// Find the leftmost slice of the sprite and the rightmost for drawing it to the screens
			int leftOfSprite{ x - halfSpriteSize };
			int rightOfSprite{ x + halfSpriteSize };

			Texture* textureFacing{ sprites[i].facingTexture(sprites[i].angleToSprite(playerX, playerY)) };

			int minBetweenRightOfSpriteAndWidth{ std::min(rightOfSprite, width) };
			int minBetweenBottomOfSpriteAndHeight{ std::min(bottomOfSprite, height) };
			for (int screenX{ std::max(leftOfSprite, 0) }; screenX < minBetweenRightOfSpriteAndWidth; screenX++)
			{
				// Normalize the x-coordinate
				float normX{ (screenX - leftOfSprite) / static_cast<float>(spriteSize) };
				// Calculate the x-coordinate for the texture
				int textureX{ static_cast<int>(normX * textureFacing->m_width) };

				// If there is no wall slice closer to the player than the sprite slice, draw the sprite slice
				if (zBuffer[screenX] > distance)
				{
					for (int y{ std::max(topOfSprite, 0) }; y < minBetweenBottomOfSpriteAndHeight; y++)
					{
						// Normalize the y-coordinate
						float normY{ (y - topOfSprite) / static_cast<float>(spriteSize) };
						// Calculate the y-coordinate for the texture
						int textureY{ static_cast<int>(normY * textureFacing->m_height) };

						// Get the color from the sprite object
						uint32_t color{ (*textureFacing)[textureY * textureFacing->m_width + textureX] };

						// In the Wolfenstein textures, the transparent parts have a black color. Don't draw completely black pixels
						if (color != 255)
							screen[y * width + screenX] = color;
					}
				}
			}
		}

		// Update the texture that will be drawn to the screen with the array of pixels
		SDL_UpdateTexture(frameBuffer, NULL, screen, width * sizeof(uint32_t));

		// Change render draw color to black
		SDL_SetRenderDrawColor(renderTarget, 0, 0, 0, 255);

		// Clear the screen
		SDL_RenderClear(renderTarget);

		if (DEBUG)
		{
			// Draw a the map to the right
			SDL_SetRenderDrawColor(renderTarget, 255, 255, 255, 255);
			for (int x{ 0 }; x < gridWidth; x++)
			{
				for (int y{ 0 }; y < gridHeight; y++)
				{
					if (gridMap[y * gridWidth + x] == '#')
					{
						SDL_Rect r{ x * (height / gridWidth) + width, y * (height / gridHeight), height / gridWidth, height / gridHeight };
						SDL_RenderDrawRect(renderTarget, &r);
					}
				}
			}

			// Calculate the position of the player from a 640 x 640 grid of pixels to a 400 x 400 grid of pixels
			float normX{ playerX / (gridSize * gridWidth) * height + width };
			float normY{ playerY / (gridSize * gridHeight) * height };

			// Draw the player
			SDL_FRect player{ normX - 5.0f, normY - 5.0f, 10, 10 };
			SDL_RenderDrawRectF(renderTarget, &player);

			// Draw direction the player is looking
			SDL_RenderDrawLineF(renderTarget, normX, normY, normX + xSpeed * 1.0f, normY + ySpeed * 1.0f);

			// Draw the horizontal intersecting rays
			//SDL_SetRenderDrawColor(renderTarget, 255, 255, 255, 0);
			//for (int i{ 0 }; i < aPoints.size(); i++)
			//{
			//	float normPointX{ aPoints[i].x / (gridSize * gridWidth) * height + width };
			//	float normPointY{ aPoints[i].y / (gridSize * gridHeight) * height };
			//	SDL_RenderDrawLineF(renderTarget, normX, normY, normPointX, normPointY);
			//}

			//// Draw the vertically intersection rays
			//for (int i{ 0 }; i < bPoints.size(); i++)
			//{
			//	float normPointX{ bPoints[i].x / (gridSize * gridWidth) * height + width };
			//	float normPointY{ bPoints[i].y / (gridSize * gridHeight) * height };
			//	SDL_RenderDrawLineF(renderTarget, normX, normY, normPointX, normPointY);
			//}

			// Draw the rays which are used to render the scene
			SDL_SetRenderDrawColor(renderTarget, 255, 0, 0, 0);
			for (int i{ 0 }; i < actualPoints.size(); i++)
			{
				float normPointX{ actualPoints[i].x / (gridSize * gridWidth) * height + width };
				float normPointY{ actualPoints[i].y / (gridSize * gridHeight) * height };
				SDL_RenderDrawLineF(renderTarget, normX, normY, normPointX, normPointY);
			}

			// Draws the points from the map which are used for floor casting. Very slow!
			/*SDL_SetRenderDrawColor(renderTarget, 255, 0, 0, 0);
			for (int i{ 0 }; i < floorPoints.size(); i++)
			{
				float normPointX{ floorPoints[i].x / (gridSize * gridWidth) * height + width };
				float normPointY{ floorPoints[i].y / (gridSize * gridHeight) * height };
				SDL_RenderDrawLineF(renderTarget, normX, normY, normPointX, normPointY);
			}*/

			// Draw FOV
			SDL_SetRenderDrawColor(renderTarget, 0, 255, 0, 255);
			SDL_RenderDrawLineF(renderTarget, normX, normY, normX + (100 * cos(radians(theta - FOV / 2.0f))), normY - (100 * sin(radians(theta - FOV / 2.0f))));
			SDL_RenderDrawLineF(renderTarget, normX, normY, normX + (100 * cos(radians(theta + FOV / 2.0f))), normY - (100 * sin(radians(theta + FOV / 2.0f))));
		}

		// Copy the rendered scene to the screen
		SDL_Rect halfScreen{ 0, 0, width, height };
		SDL_RenderCopy(renderTarget, frameBuffer, NULL, &halfScreen);

		SDL_RenderPresent(renderTarget);

	}


	SDL_DestroyWindow(win);				// Deallocates window memory + winSurface
	SDL_DestroyRenderer(renderTarget);	// Deallocates the renderer
	SDL_DestroyTexture(frameBuffer);

	SDL_Quit();
	IMG_Quit();
	TTF_Quit();
	Mix_Quit();

	delete[] screen;

	return 0;
}
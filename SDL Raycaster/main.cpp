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
std::vector<int> wallHeights(gridWidth * gridHeight);	// Stores the wall height in each grid spot

Texture* wallTextureLocations[gridWidth * gridHeight];
Texture* floorTextureLocations[gridWidth * gridHeight];
Texture* ceilingTextureLocations[gridWidth * gridHeight];

// Vector which holds all the sprites
std::vector<Sprite> sprites{};

int FOV{ 90 };							// Field of view of player
int distanceToProjectionPlane{ 277 };	// Distance of the "camera" (player) to the "projection plane" (screen)

int playerHeight{ gridSize / 2 };		// Height of player (typically half of gridSize)
int playerRadius{ 15 };					// Radius of player

// The "adjusted" distance to the projection plane, used so that I can adjust the field of view
float adjustedDistanceToProjectionPlane{ (width / 2) / fabs(tanf((FOV / 2) * (M_PI / 180.0f))) };

float theta{ -49.175f };					// Angle of the player
float playerX{ 64.0f };				// x-coordinate of the player in pixels, not grid coordinates
float playerY{ 64.0f };				// y-coordinate of the player in pixels, not grid coordinates
float playerSpeed{ 100.0f };			// Speed at which the player moves
float playerTurnSpeed{ 75.0f };			// Speed with which the player can turn
float playerLookUpSpeed{ 175.0f };		// Speed with which the player can look up and down (modifies projectionPlaneCenter variable)

// Used to calculate the time elapsed between frames
float previousTime{};
float currentTime{};
float deltaTime{};
float FPS{};

struct Point
{
	float x;
	float y;
};

std::vector<Point> aPoints{};
std::vector<Point> bPoints{};
std::vector<Point> vPoints{};

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

float clamp(float x, float lo, float hi)
{
	if (x > hi)
		return hi;

	if (x < lo)
		return lo;

	return x;
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

	Texture* wallTexture{ &colorstone };
	Texture* floorTexture1{ &greystone };
	Texture* floorTexture2{ &bluestone };
	Texture* ceilingTexture{ &wood };

	float sumOfFPS{ 0.0f };
	int numFrames{ 0 };

	// Add sprites to the sprite array
	/*sprites.push_back(Sprite{ &spriteTexture, gridSize * 5.5f, gridSize * 17.5f });
	sprites.push_back(Sprite{ &spriteTexture, 350.0f + 127.0f, 350.0f });
	sprites.push_back(Sprite{ &spriteTexture2, 11.5f * gridSize, 13.5f * gridSize });
	sprites.push_back(Sprite{ &spriteTexture2, 18.5f * gridSize, 13.5f * gridSize });
	sprites.push_back(Sprite{ {&front, &right, &back, &left}, 16.0f * gridSize, 5.0f * gridSize, 0.0f });*/

	// Create the map
	wallHeights = {
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 0,	0,	0,	0,	64,	0,	0,	0,	64, 64, 0,	0,	0,	0,	45,	0,	0,	0,	64,
		64, 0,	0,	0,	0,	64,	0,	0,	0,	64, 64, 0,	0,	0,	0,	40,	0,	0,	0,	64,
		64, 0,	0,	0,	0,	64,	0,	0,	0,	64, 64, 0,	0,	0,	0,	35,	0,	0,	0,	64,
		64, 0,  64,	0,	0,	64,	0,	0,	0,	16, 64, 0,  64,	0,	0,	30,	0,	0,	0,	64,
		64, 0,	0,	64,	0,	64,	0,	0,	0,	16, 64, 0,	0,	64,	0,	25,	0,	0,	0,	64,
		64, 0,	0,	0,	0,	64,	0,	0,	0,	16, 16, 0,	0,	0,	0,	20,	0,	0,	0,	64,
		64, 0,	0,	0,	0,	64,	0,	0,	0,	64, 64, 0,	0,	0,	0,	15,	0,	0,	0,	64,
		64, 0,	0,	0,	0,	64,	0,	0,	0,	64, 64, 0,	0,	0,	0,	10,	0,	0,	0,	64,
		64, 0,	0,	0,	0,	0,	0,	0,	0,	 0,  0, 0,	0,	0,	0,	0,	0,	0,	0,	64,
		64, 0,	0,	0,	0,	0,	0,	0,	0,	64, 64, 0,	0,	0,	0,	0,	0,	0,	0,	64,
		64, 0,	0,	0,	0,	0,	0,	0,	0,	64, 64, 0,	0,	0,	0,	0,	0,	0,	0,	64,
		64, 0,	0,	0,	0,	0,	0,	0,	0,	64, 64, 0,	0,	0,	0,	0,	0,	0,	0,	64,
		64, 0,	0,	0,	0,	0,	0,	0,	0,	64, 64, 0,	0,	0,	0,	0,	0,	0,	0,	64,
		64, 0,	0,	0,	0,	0,	0,	0,	0,	64, 64, 0,	0,	0,	0,	0,	0,	0,	0,	64,
		64, 0,	0,	0,	0,	0,	0,	0,	0,	64, 64, 0,	0,	0,	0,	0,	0,	0,	0,	64,
		64, 0,  32, 16,	0,	0,	0,	0,	0,	64, 64, 0,  64, 64,	0,	0,	0,	0,	0,	64,
		64, 0,  48,	32,	0,	0,	0,	0,	0,	64, 64, 0,  64,	64,	0,	0,	0,	0,	0,	64,
		64, 0,	0,	0,	0,	0,	0,	0,	0,	64, 64, 0,	0,	0,	0,	0,	0,	0,	0,	64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	};

	// Create a string map
	for (int y{ 0 }; y < gridHeight; y++)
	{
		for (int x{ 0 }; x < gridWidth; x++)
		{
			if (wallHeights[y * gridWidth + x] == 0)
			{
				gridMap += '-';
			}
			else
			{
				gridMap += '#';
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

		// Movement + collision
		
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
		/*
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
		*/
		
		// Precalculate some values that will be used in the loops below
		float cosOfThetaMinusHalfFOV{ cos(radians(theta - FOV * 0.5f)) };
		float sinOfThetaMinusHalfFOV{ sin(radians(theta - FOV * 0.5f)) };

		float cosOfThetaPlusHalfFOV{ cos(radians(theta + FOV * 0.5f)) };
		float sinOfThetaPlusHalfFOV{ sin(radians(theta + FOV * 0.5f)) };

		float cosOfHalfFOV{ cos(radians(FOV * 0.5f)) };
		/*
		// Draw Floor
		// Loop over every horizontal line from the center of the projection plane down. Exclude the line at the center because
		// it would cause a divide by 0 error
		for (int y{ projectionPlaneCenter + 1 }; y < height; y++)
		{
			// Calculate how far the strip of floor corresponding to the horizontal line on the projection plane is from the player
			float distanceToStripe{ (distanceToProjectionPlane * static_cast<float>(playerHeight)) / (y - projectionPlaneCenter) };

			// Calculate the corrected distance (the walls are calculated with the corrected distance, so the floor must be as well)
			float correctedDistance{ distanceToStripe / cosOfHalfFOV };

			// Calculate the point on the horizontal stripe to the far left of the FOV
			float adX{ correctedDistance * cosOfThetaMinusHalfFOV };
			float adY{ correctedDistance * -sinOfThetaMinusHalfFOV };

			float aX{ playerX + adX };
			float aY{ playerY + adY };

			// aPoints.push_back(Point{ aX, aY });

			// Calculate the point on the horizontal stripe to the far right of the FOV
			float bdX{ correctedDistance * cosOfThetaPlusHalfFOV };
			float bdY{ correctedDistance * -sinOfThetaPlusHalfFOV };

			float bX{ playerX + bdX };
			float bY{ playerY + bdY };

			// bPoints.push_back(Point{ bX, bY });

			// Calculate a vector that points from point A to point B
			float vX{ bX - aX };
			float vY{ bY - aY };

			// Divide the vector by the width of the screen so that each time the vector is added to point A, a new pixel 
			// on the screen is represented
			float floorStepX{ vX / width };
			float floorStepY{ vY / width };

			Texture* floorTexture{};

			// A for loop that goes accross the horizontal line
			for (int x{ width - 1 }; x >= 0; x--)
			{

				// vPoints.push_back(Point{ aX, aY });

				// Calculate the coordinates of the grid square the point on the floor is in
				int gridX{ static_cast<int>(aX / gridSize) * gridSize };
				int gridY{ static_cast<int>(aY / gridSize) * gridSize };

				/*float num{ gridX * gridY / static_cast<float>(width * height) };
				unsigned int color{ static_cast<unsigned int>(0xffffffff * num) };
				screen[y * width + x] = color;
				

				if ((gridX / gridSize + gridY / gridSize) % 2 == 0)
					floorTexture = floorTexture1;
				else
					floorTexture = floorTexture2;

				// Calculate the normalized coordinates of the point in terms of the grid square it is in
				float normX{ (static_cast<int>(aX) - gridX) / static_cast<float>(gridSize) };
				float normY{ (static_cast<int>(aY) - gridY) / static_cast<float>(gridSize) };

				// Calculate where those normalized coordinates fall on the texture of the floor
				int textureX{ static_cast<int>(normX * floorTexture->m_width) };
				int textureY{ static_cast<int>(normY * floorTexture->m_height) };

				// Write the color to the screen array
				if (textureY * floorTexture->m_width + textureX >= 0 && textureY * floorTexture->m_width + textureX < floorTexture->m_height * floorTexture->m_width)
				{
					screen[y * width + x] = (*floorTexture)[textureY * floorTexture->m_width + textureX];
				}

				// Increment point A to get the next point on the floor
				aX += floorStepX;
				aY += floorStepY;
			}
		}
		*/

		// Draw Ceiling
		// Loop over every horizontal line from the center of the projection plane down. Exclude the line at the center because
		// it would cause a divide by 0 error
		for (int y{ projectionPlaneCenter - 1 }; y >= 0; y--)
		{
			// Calculate how far the strip of ceiling corresponding to the horizontal line on the projection plane is from the player
			float distanceToStripe{ (distanceToProjectionPlane * static_cast<float>(gridSize - playerHeight)) / (projectionPlaneCenter - y) };

			// Calculate the corrected distance (the walls are calculated with the corrected distance, so the ceiling must be as well)
			float correctedDistance{ distanceToStripe / cosOfHalfFOV };

			// Calculate the point on the horizontal stripe to the far left of the FOV
			float adX{ correctedDistance * cosOfThetaMinusHalfFOV };
			float adY{ correctedDistance * -sinOfThetaMinusHalfFOV };

			float aX{ playerX + adX };
			float aY{ playerY + adY };

			// aPoints.push_back(Point{ aX, aY });

			// Calculate the point on the horizontal stripe to the far right of the FOV
			float bdX{ correctedDistance * cosOfThetaPlusHalfFOV };
			float bdY{ correctedDistance * -sinOfThetaPlusHalfFOV };

			float bX{ playerX + bdX };
			float bY{ playerY + bdY };

			// bPoints.push_back(Point{ bX, bY });

			// Calculate a vector that points from point A to point B
			float vX{ bX - aX };
			float vY{ bY - aY };

			// Divide the vector by the width of the screen so that each time the vector is added to point A, a new pixel 
			// on the screen is represented
			float floorStepX{ vX / width };
			float floorStepY{ vY / width };

			// A for loop that goes accross the horizontal line
			for (int x{ width - 1 }; x >= 0; x--)
			{

				// vPoints.push_back(Point{ aX, aY });

				// Calculate the coordinates of the grid square the point on the ceiling is in
				int gridX{ static_cast<int>(aX / gridSize) * gridSize };
				int gridY{ static_cast<int>(aY / gridSize) * gridSize };

				/*
				float num{ gridX * gridY / static_cast<float>(width * height) };
				unsigned int color{ static_cast<unsigned int>(0xffffffff * num) };
				screen[y * width + x] = color;
				*/

				// Calculate the normalized coordinates of the point in terms of the grid square it is in
				float normX{ (static_cast<int>(aX) - gridX) / static_cast<float>(gridSize) };
				float normY{ (static_cast<int>(aY) - gridY) / static_cast<float>(gridSize) };

				// Calculate where those normalized coordinates fall on the texture of the ceiling
				int textureX{ static_cast<int>(normX * ceilingTexture->m_width) };
				int textureY{ static_cast<int>(normY * ceilingTexture->m_height) };

				// Write the color to the screen array
				if (textureY * ceilingTexture->m_width + textureX >= 0 && textureY * ceilingTexture->m_width + textureX < ceilingTexture->m_height * ceilingTexture->m_width)
				{
					screen[y * width + x] = (*ceilingTexture)[textureY * ceilingTexture->m_width + textureX];
				}

				// Increment point A to get the next point on the floor
				aX += floorStepX;
				aY += floorStepY;
			}
		}

		// Send a ray out into the scene for each vertical row of pixels in the screen array
		for (int x{ 0 }; x < width; x++)
		{
			// Calculate the angle between two rays
			float angleBetween{ degrees(atanf(static_cast<float>(x - (width / 2)) / adjustedDistanceToProjectionPlane)) };

			// Find the angle of the ray
			float rayAngle{ theta - angleBetween };

			// Precalculate some values that will be used in the floor and ceiling casting loops below
			float cosOfRayAngle{ cosf(radians(rayAngle)) };
			float sinOfRayAngle{ sinf(radians(rayAngle)) };
			float cosOfThetaMinusRayAngle{ cosf(radians(theta - rayAngle)) };

			int currentHeight{ wallHeights[static_cast<int>(playerY / gridSize) * gridWidth + static_cast<int>(playerX / gridSize)] };

			int previousTopOfWall{ height };	// Saves the position of the top of the previous wall on the projection plane to draw the floor on top of that wall
			int previousWallHeight{ currentHeight };		// Saves the height of the previous wall to calculate where the next wall begins on the projection plane

			bool finished{ false };

			float light{ 255.0f };

			// Distance from the player to the current intersection with the grid (this version of the raycasters checks every intersection 
			// between the ray and the grid)
			float distance{ 0.0f };

			// A number on the interval 0 < gridSpaceColumn < gridSize. The column on the grid where the ray intersects the wall
			int gridSpaceColumn{};

			// Precalculate a value that will be used more times
			float tanOfRayAngle{ tanf(radians(rayAngle)) };

			// Find the angle of the ray on the interval 0 <= rayAngle < 360
			rayAngle = getCoterminalAngle(rayAngle);

			if (rayAngle == 360.0f)
				rayAngle = 0.0f;

			// These two boolean values are used to determine how to texture the wall by determining which side of the wall the ray hit
			bool topOrBottom{};	// True means the ray hit the top of the wall, false means it hit the bottom
			bool leftOrRight{};	// True means the ray hit the left of the wall, false means it hit the right

			// Point I is the intersection point (chosen between A and B)
			float iX{};
			float iY{};

			// CALCULATE HORIZONTAL INTERSECTIONS

			// Distance to intersection with horizontal grid line
			float horizontalDistance{ -1.0f };

			// Point A is the point of the first intersection between the ray and the horizontal grid lines
			float aX{};
			float aY{};

			// The change between point A and the next intersection with the horizontal grid lines
			float adX{};
			float adY{};

			// If the ray is facing up... (or downwards on the coordinate grid)
			if (rayAngle < 180)
			{
				// Because the ray is pointing up, that means it will hit the bottoms of the wall
				topOrBottom = false;

				// The first intersection will be part of the grid below (calculates y-coordinate of grid line below)
				aY = floorf(playerY / static_cast<float>(gridSize)) * gridSize;

				// The next intersection with a horizontal grid line will be gridSize units below
				adY = -static_cast<float>(gridSize);

				//	      90					90		
				//  -x,-y | +x,-y			-tan | +tan
				// 180 ---+--- 0		  180 ---+--- 0	
				//	-x,+y |	+x,+y			+tan | -tan
				//		 270				    270		
				// When rayAngle < 90, dx should be >0, and when rayAngle > 90, dx should be <0
				// It just so happens that tan is >0 when rayAngle < 90 degrees, and tan is <0 when rayAngle > 90
				// so I don't have to change the signs at all
				adX = gridSize / tanOfRayAngle;

				// Calculate the x-coordinate of the first intersection with a horizontal gridline
				aX = playerX - (aY - playerY) / tanOfRayAngle;

				// Make part of the grid below for ease of checking for a wall
				aY -= 0.001f;
			}
			// If ray is facing down... (or upwards on the coordinate grid)
			else
			{
				// The ray is facing down, so the ray hits the top of the wall
				topOrBottom = true;

				// The first horizontal grid intersection is with the grid line above the player
				aY = floorf(playerY / static_cast<float>(gridSize)) * gridSize + gridSize;

				// The next gridline with be gridSize units above the player
				adY = static_cast<float>(gridSize);

				//	      90					90		
				//  -x,-y | +x,-y			-tan | +tan
				// 180 ---+--- 0		  180 ---+--- 0	
				//	-x,+y |	+x,+y			+tan | -tan
				//		 270				    270		
				// When rayAngle < 270, dx should be <0, and when rayAngle > 270, dx should be >0
				// It just so happens that tan is >0 when rayAngle < 270 degrees, and tan is <0 when rayAngle > 270
				// so I have to flip the signs with the negative
				adX = -gridSize / tanOfRayAngle;

				// Calculate the x-coordinate of the first intersection with a horizontal gridline
				aX = playerX - (aY - playerY) / tanOfRayAngle;
			}

			// CALCULATE VERTICAL INTERSECTIONS (very similar to calculating horizontal intersections)

			// Distance to intersection with vertical grid line
			float verticalDistance{ -1.0f };

			// Point B is the point of the first intersection between the ray and the vertical grid lines
			float bX{};
			float bY{};

			// The change between point B and the next intersection with the vertical grid lines
			float bdX{};
			float bdY{};

			// If the ray is facing to the right...
			if (rayAngle < 90.0f || rayAngle > 270.0f)
			{
				// The ray is facing to the right, so it will hit the left wall
				leftOrRight = true;

				// The first intersection will be in a grid to the right of the current grid
				bX = floorf(playerX / gridSize) * gridSize + gridSize;

				// The ray is moving in a positive x-direction
				bdX = static_cast<float>(gridSize);

				//	      90					90		
				//  -x,-y | +x,-y			-tan | +tan
				// 180 ---+--- 0		  180 ---+--- 0	
				//	-x,+y |	+x,+y			+tan | -tan
				//		 270				    270		
				// When rayAngle < 180, dy should be <0, and when rayAngle > 180, dy should be >0
				// It just so happens that tan is >0 when rayAngle < 180 degrees, and tan is <0 when rayAngle > 180
				// so I have to flip the signs with the negative
				bdY = -tanOfRayAngle * gridSize;

				// Calculate the y-coordinate of the first intersection with a vertical gridline
				bY = playerY + (playerX - bX) * tanOfRayAngle;
			}
			// If the ray is facing to the left...
			else
			{
				// The ray is facing left so it will hit the wall to the right
				leftOrRight = false;

				// The first intersection will be in a grid to the left
				bX = floorf(playerX / gridSize) * gridSize;

				// The ray is moving in a negative x-direction
				bdX = -static_cast<float>(gridSize);

				//	      90					90		
				//  -x,-y | +x,-y			-tan | +tan
				// 180 ---+--- 0		  180 ---+--- 0	
				//	-x,+y |	+x,+y			+tan | -tan
				//		 270				    270		
				// When rayAngle < 180, dy should be <0, and when rayAngle > 180, dy should be >0
				// It just so happens that tan is <0 when rayAngle < 180 degrees, and tan is >0 when rayAngle > 180
				// so I don't have to change the signs at all
				bdY = tanOfRayAngle * gridSize;

				// Calculate the y-coordinate of the first intersection with a vertical gridline
				bY = playerY + (playerX - bX) * tanOfRayAngle;

				bX -= 0.001f;
			}

			// Determine which initial point (point A or point B) is closer to the player
			horizontalDistance = (playerX - aX) * (playerX - aX) + (playerY - aY) * (playerY - aY);
			verticalDistance = (playerX - bX) * (playerX - bX) + (playerY - bY) * (playerY - bY);

			// If the intersection with the horizontal grid lines is closer...
			if (horizontalDistance < verticalDistance)
			{
				// The distance is equal to the horizontal distance
				distance = sqrt(horizontalDistance);

				// The first intersection is equal to the horizontal intersection
				iX = aX;
				iY = aY;

				// Subtract the vertical change in x and y from the first vertical intersection. This is so that in the while
				// loop, when I am checking to see if the horizontal or vertical intersection is closer, it will consider the first vertical 
				// intersection rather than the one after
				bX -= bdX;
				bY -= bdY;

				light = 255.0f;

				// If the ray hit the top of the wall...
				if (topOrBottom)
				{
					// First column on the top of the wall; at the top left corner of the wall
					int gridX{ static_cast<int>(iX / gridSize) * gridSize + (gridSize - 1) };
					gridSpaceColumn = gridX - static_cast<int>(iX);
				}
				// Otherwise it hit the bottom of the wall
				else
				{
					// First column on the bottom of the wall; at the bottom right corner of the wall
					int gridX{ static_cast<int>(iX / gridSize) * gridSize };
					gridSpaceColumn = static_cast<int>(iX) - gridX;
				}
			}
			// Otherwise the intersection with the vertical grid lines is closer
			else
			{
				// The distance is equal to the vertical distance
				distance = sqrt(verticalDistance);

				// The first intersection is the vertical intersection
				iX = bX;
				iY = bY;

				// Subtract the horizontal change in x and y from the first horizontal intersection. This is so that in the while
				// loop, when I am checking to see if the horizontal or vertical intersection is closer, it will consider the first horizontal 
				// intersection rather than the one after
				aX -= adX;
				aY -= adY;

				light = 128.0f;

				// If the ray hit the left side of the wall...
				if (leftOrRight)
				{
					// First column on the left side of the wall; at the top left corner of the wall
					int gridY{ static_cast<int>(iY / gridSize) * gridSize };
					gridSpaceColumn = static_cast<int>(iY) - gridY;
				}
				// Otherwise it hit the right side of the wall
				else
				{
					// First column on the right side of the wall; at the bottom right corner of the wall
					int gridY{ static_cast<int>(iY / gridSize) * gridSize + (gridSize - 1) };
					gridSpaceColumn = gridY - static_cast<int>(iY);
				}
			}

			// The next 30 lines of code draw the walls of the grid the player is standing in; without these lines of code,
			// when the player stepped into a square with a wall in it, the walls would disappear
			
			// Determine the grid square the player is standing in
			int playerGridX{ static_cast<int>(playerX / gridSize) };
			int playerGridY{ static_cast<int>(playerY / gridSize) };

			// Determine the height of the floor beneath the player
			int heightBeneathPlayer{ wallHeights[playerGridY * gridWidth + playerGridX] };

			// Calculate the fisheye corrected distance used for rendering. All the variables below are prefaced by player
			// to distinguish them from the variables in the main rendering loop, not because they represent an attribute of the player
			float playerRenderingDistance{ distance * cosf(radians(theta - rayAngle)) };

			// Calculate the wall height in the player's grid square
			int playerWallHeight{ static_cast<int>((distanceToProjectionPlane / playerRenderingDistance) * heightBeneathPlayer) + 1 };

			// Calculate the top and the bottom of the wall
			int playerBottomOfWall{ static_cast<int>(projectionPlaneCenter + (distanceToProjectionPlane * playerHeight) / playerRenderingDistance) };
			int playerTopOfWall{ playerBottomOfWall - playerWallHeight };

			// Calculate the texture column for the wall slice
			int playerTextureSpaceColumn{ static_cast<int>(static_cast<float>(gridSpaceColumn) / gridSize * wallTexture->m_width) };

			int minBetweenHeightAndPlayerBottomOfWall{ std::min(height, playerBottomOfWall) };

			// Draw the wall sliver
			for (int y{ std::max(playerTopOfWall, 0) }; y < minBetweenHeightAndPlayerBottomOfWall; y++)
			{
				// The row on the texture
				int textureSpaceRow{ static_cast<int>((y - playerTopOfWall) / static_cast<float>(playerWallHeight) * wallTexture->m_height) };

				// Get the color of the texture at the point on the wall (x, y)
				uint32_t color{ (*wallTexture)[textureSpaceRow * wallTexture->m_width + playerTextureSpaceColumn] };

				screen[y * width + x] = calculateLighting(color, light);
				// screen[y * width + x] = color;
			}

			if (playerHeight > heightBeneathPlayer)
			{
				for (int y{ height - 1 }; y >= playerTopOfWall; y--)
				{
					// Get the distance from the screen pixel to the point on the floor that it contains
					float floorDistance{ static_cast<float>((playerHeight - heightBeneathPlayer) * distanceToProjectionPlane) / (y - projectionPlaneCenter) };

					// Correct for the fish eye effect
					floorDistance /= cosf(radians(theta - rayAngle));

					// Calculate the point on the floor that contains the color of the pixel on the screen
					float floorX{ playerX + floorDistance * cosf(radians(rayAngle)) };
					float floorY{ playerY + floorDistance * -sinf(radians(rayAngle)) };

					// Keep the point within the bounds of the map to avoid accessing the floor texture in an improper way
					floorX = clamp(floorX, 0.0f, gridWidth * gridSize);
					floorY = clamp(floorY, 0.0f, gridWidth * gridSize);

					// Calculate the grid square the floor is in
					int floorGridX{ static_cast<int>(floorX / gridSize) * gridSize };
					int floorGridY{ static_cast<int>(floorY / gridSize) * gridSize };

					// Calculate the texture coordinates that correspond to the point on the floor
					float normX{ (static_cast<int>(floorX) - floorGridX) / static_cast<float>(gridSize) };
					float normY{ (static_cast<int>(floorY) - floorGridY) / static_cast<float>(gridSize) };

					int textureX{ static_cast<int>(normX * floorTexture1->m_width) };
					int textureY{ static_cast<int>(normY * floorTexture1->m_height) };

					int i{ textureY * floorTexture1->m_width + textureX };
					uint32_t color{};

					if (i < floorTexture1->m_width * floorTexture1->m_height)
						color = (*floorTexture1)[i];
					else
						color = 0x00FFFF00;

					screen[y * width + x] = color;
				}
			}
			else
			{
				/*
				for (int y{ playerTopOfWall }; y >= 0; y--)
				{
					// Get the distance from the screen pixel to the point on the floor that it contains
					float floorDistance{ static_cast<float>(((gridSize - heightBeneathPlayer) - playerHeight) * distanceToProjectionPlane) / (projectionPlaneCenter - y) };

					// Correct for the fish eye effect
					floorDistance /= cosf(radians(theta - rayAngle));

					// Calculate the point on the floor that contains the color of the pixel on the screen
					float floorX{ playerX + floorDistance * cosf(radians(rayAngle)) };
					float floorY{ playerY + floorDistance * -sinf(radians(rayAngle)) };

					// Keep the point within the bounds of the map to avoid accessing the floor texture in an improper way
					floorX = clamp(floorX, 0.0f, gridWidth * gridSize);
					floorY = clamp(floorY, 0.0f, gridWidth * gridSize);

					// Calculate the grid square the floor is in
					int floorGridX{ static_cast<int>(floorX / gridSize) * gridSize };
					int floorGridY{ static_cast<int>(floorY / gridSize) * gridSize };

					// Calculate the texture coordinates that correspond to the point on the floor
					float normX{ (static_cast<int>(floorX) - floorGridX) / static_cast<float>(gridSize) };
					float normY{ (static_cast<int>(floorY) - floorGridY) / static_cast<float>(gridSize) };

					int textureX{ static_cast<int>(normX * floorTexture1->m_width) };
					int textureY{ static_cast<int>(normY * floorTexture1->m_height) };

					int i{ textureY * floorTexture1->m_width + textureX };
					uint32_t color{};

					if (i < floorTexture1->m_width * floorTexture1->m_height)
						color = (*floorTexture1)[i];
					else
						color = 0x00FFFF00;

					screen[y * width + x] = color;
				}
				*/
				finished = true;
			}

			// Update previousTopOfWall
			previousTopOfWall = playerTopOfWall;

			// Find all intersections between the ray and the grid, and render the wall of the given height
			while (!finished)
			{
				// Calculate which grid square the intersection belongs to
				int intersectionGridX{ static_cast<int>(iX / gridSize) };
				int intersectionGridY{ static_cast<int>(iY / gridSize) };

				float num{ intersectionGridX * intersectionGridY / static_cast<float>(gridWidth * gridHeight) };
				// unsigned int color{ static_cast<unsigned int>(0xffffffff * num) };
				

				// Look up the height of the wall at those grid coordinates
				int variableHeight{ wallHeights[intersectionGridY * gridWidth + intersectionGridX] };

				// Correct fish-eye distortion for the actual rendering of the walls
				float renderingDistance{ distance * cosf(radians(theta - rayAngle)) };

				// I use the fisheye corrected distance because the comparison in the sprite rendering loop uses the fisheye corrected distance
				// to the sprite
				// zBuffer[x] = distance;

				// The code up until the update of previousTopOfWall renders the front faces of the walls
				
				// Calculate the height of the wall
				int wallHeight{ static_cast<int>((distanceToProjectionPlane / renderingDistance) * variableHeight) + 1};

				// Y-coordinates of the bottom and top of the wall. Calculated in terms of player height and projection plane center (using similar
				// triangles) so that when the player height changes, the location of the wall will as well
				int bottomOfWall{ static_cast<int>(projectionPlaneCenter + (distanceToProjectionPlane * playerHeight) / renderingDistance) };
				int topOfWall{ bottomOfWall - wallHeight };

				if (bottomOfWall > previousTopOfWall)
					bottomOfWall = previousTopOfWall;

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

					screen[y * width + x] = calculateLighting(color, light);
					// screen[y * width + x] = color;
				}

				// Update previousWallHeight now that its purpose has been fulfilled
				previousWallHeight = variableHeight;

				// Update previousTopOfWall, but only if the current wall is larger than the previous (a larger wall will have 
				// a smaller topOfWall variable value)
				previousTopOfWall = std::min(previousTopOfWall, topOfWall);

				// The code until backRenderingDistance is instantiated updates the grid intersection

				// Determine which initial point (point A or point B) is closer to the player
				horizontalDistance = (playerX - (aX + adX)) * (playerX - (aX + adX)) + (playerY - (aY + adY)) * (playerY - (aY + adY));
				verticalDistance = (playerX - (bX + bdX)) * (playerX - (bX + bdX)) + (playerY - (bY + bdY)) * (playerY - (bY + bdY));

				if (horizontalDistance < verticalDistance)
				{
					distance = sqrt(horizontalDistance);
					aX += adX;
					aY += adY;
					iX = aX;
					iY = aY;

					light = 255.0f;

					// If the ray hit the top of the wall...
					if (topOrBottom)
					{
						// First column on the top of the wall; at the top left corner of the wall
						int gridX{ static_cast<int>(iX / gridSize) * gridSize + (gridSize - 1) };
						gridSpaceColumn = gridX - static_cast<int>(iX);
					}
					// Otherwise it hit the bottom of the wall
					else
					{
						// First column on the bottom of the wall; at the bottom right corner of the wall
						int gridX{ static_cast<int>(iX / gridSize) * gridSize };
						gridSpaceColumn = static_cast<int>(iX) - gridX;
					}
				}
				else
				{
					distance = sqrt(verticalDistance);
					bX += bdX;
					bY += bdY;
					iX = bX;
					iY = bY;

					light = 128.0f;

					// If the ray hit the left side of the wall...
					if (leftOrRight)
					{
						// First column on the left side of the wall; at the top left corner of the wall
						int gridY{ static_cast<int>(iY / gridSize) * gridSize };
						gridSpaceColumn = static_cast<int>(iY) - gridY;
					}
					// Otherwise it hit the right side of the wall
					else
					{
						// First column on the right side of the wall; at the bottom right corner of the wall
						int gridY{ static_cast<int>(iY / gridSize) * gridSize + (gridSize - 1) };
						gridSpaceColumn = gridY - static_cast<int>(iY);
					}
				}

				// Code up until after the for loop projects and renders the back faces of the wall. This process uses the same
				// methods as rendering the front faces, but it is done with the updated intersection and distance information.
				// However, instead of using the next grid square, it uses the same grid square that was used to render the front faces.

				float backRenderingDistance{ distance * cos(radians(theta - rayAngle)) };

				int backWallHeight{ static_cast<int>((distanceToProjectionPlane / backRenderingDistance) * variableHeight) };

				int backBottomOfWall{ static_cast<int>(projectionPlaneCenter + (distanceToProjectionPlane * playerHeight) / backRenderingDistance) };
				int backTopOfWall{ backBottomOfWall - backWallHeight };

				if (backBottomOfWall > previousTopOfWall)
					backBottomOfWall = previousTopOfWall;

				// It is not necessary to render the back faces when the floors are being rendered
				/*
				// The column on the texture which corresponds to the position of the ray intersection with the wall
				int backTextureSpaceColumn{ static_cast<int>(static_cast<float>(gridSpaceColumn) / gridSize * wallTexture->m_width) };

				int minBetweenHeightAndBackBottomOfWall{ std::min(height, backBottomOfWall) };
				
				// Draw the wall sliver
				for (int y{ std::max(backTopOfWall, 0) }; y < minBetweenHeightAndBackBottomOfWall; y++)
				{
					// The row on the texture
					int backTextureSpaceRow{ static_cast<int>((y - backTopOfWall) / static_cast<float>(backWallHeight) * wallTexture->m_height) };

					// Get the color of the texture at the point on the wall (x, y)
					uint32_t color{ (*wallTexture)[backTextureSpaceRow * wallTexture->m_width + backTextureSpaceColumn] };

					screen[y * width + x] = calculateLighting(color, light);
					// screen[y * width + x] = color;
				}
				*/

				// Clip the values of topOfWall and backTopOfWall to the screen
				int minBetweenFrontTopOfWallAndHeight{ std::min(topOfWall, height - 1) };
				int maxBetweenBackTopOfWallAnd0{ std::max(backTopOfWall, 0) };

				// If the y-value at which the floor rendering starts is greater than the previous top of wall, then
				// that floor is obstructed and the y-value at which the floor rendering starts much be changed so
				// that it now starts at the top of the previous wall
				if (minBetweenFrontTopOfWallAndHeight > previousTopOfWall)
					minBetweenFrontTopOfWallAndHeight = previousTopOfWall;

				if (variableHeight != gridSize)
				{
					for (int y{ minBetweenFrontTopOfWallAndHeight}; y >= maxBetweenBackTopOfWallAnd0; y--)
					{
						// Get the distance from the screen pixel to the point on the floor that it contains
						float floorDistance{ static_cast<float>((playerHeight - variableHeight) * distanceToProjectionPlane) / (y - projectionPlaneCenter) };

						// Correct for the fish eye effect
						floorDistance /= cosf(radians(theta - rayAngle));
						
						// Calculate the point on the floor that contains the color of the pixel on the screen
						float floorX{ playerX + floorDistance * cosf(radians(rayAngle)) };
						float floorY{ playerY + floorDistance * -sinf(radians(rayAngle)) };

						// Keep the point within the bounds of the map to avoid accessing the floor texture in an improper way
						floorX = clamp(floorX, 0.0f, gridWidth * gridSize);
						floorY = clamp(floorY, 0.0f, gridWidth * gridSize);

						// Calculate the grid square the floor is in
						int floorGridX{ static_cast<int>(floorX / gridSize) * gridSize };
						int floorGridY{ static_cast<int>(floorY / gridSize) * gridSize };

						// Calculate the texture coordinates that correspond to the point on the floor
						float normX{ (static_cast<int>(floorX) - floorGridX) / static_cast<float>(gridSize) };
						float normY{ (static_cast<int>(floorY) - floorGridY) / static_cast<float>(gridSize) };

						int textureX{ static_cast<int>(normX * floorTexture1->m_width) };
						int textureY{ static_cast<int>(normY * floorTexture1->m_height) };

						int i{ textureY * floorTexture1->m_width + textureX };
						uint32_t color{};

						if (i < floorTexture1->m_width * floorTexture1->m_height)
							color = (*floorTexture1)[i];
						else
							color = 0x00FFFF00;

						screen[y * width + x] = color;
					}
				}

				previousTopOfWall = std::min(previousTopOfWall, backTopOfWall);

				// If the height of the wall is the max height, then there is no need to continue finding 
				// wall intersections because this wall obstructs them from view. If the top of the wall is less
				// than 0, then the wall goes over the top of the screen. If the intersection point is outside the map,
				// then the raycasting process is finished
				if (iX < 0.0f || iX > gridSize * gridWidth || iY < 0.0f || iY > gridSize * gridHeight
					|| variableHeight == gridSize || topOfWall < 0)
				{
					finished = true;
				}
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

			SDL_SetRenderDrawColor(renderTarget, 255, 0, 0, 255);
			for (int i{ 0 }; i < aPoints.size(); i++)
			{
				float normPointX{ aPoints[i].x / (gridSize * gridWidth) * height + width };
				float normPointY{ aPoints[i].y / (gridSize * gridHeight) * height };

				SDL_RenderDrawPoint(renderTarget, normPointX, normPointY);
			}

			SDL_SetRenderDrawColor(renderTarget, 0, 255, 0, 255);
			for (int i{ 0 }; i < bPoints.size(); i++)
			{
				float normPointX{ bPoints[i].x / (gridSize * gridWidth) * height + width };
				float normPointY{ bPoints[i].y / (gridSize * gridHeight) * height };

				SDL_RenderDrawPoint(renderTarget, normPointX, normPointY);
			}

			
			SDL_SetRenderDrawColor(renderTarget, 0, 255, 128, 255);
			for (int i{ 0 }; i < vPoints.size(); i++)
			{
				float normPointX{ vPoints[i].x / (gridSize * gridWidth) * height + width };
				float normPointY{ vPoints[i].y / (gridSize * gridHeight) * height };

				SDL_RenderDrawPoint(renderTarget, normPointX, normPointY);
			}
			
			// Draw FOV
			SDL_SetRenderDrawColor(renderTarget, 0, 0, 255, 255);
			SDL_RenderDrawLineF(renderTarget, normX, normY, normX + (100 * cos(radians(theta - FOV / 2.0f))), normY - (100 * sin(radians(theta - FOV / 2.0f))));
			SDL_RenderDrawLineF(renderTarget, normX, normY, normX + (100 * cos(radians(theta + FOV / 2.0f))), normY - (100 * sin(radians(theta + FOV / 2.0f))));
		}

		// Copy the rendered scene to the screen
		SDL_Rect halfScreen{ 0, 0, width, height };
		SDL_RenderCopy(renderTarget, frameBuffer, NULL, &halfScreen);

		SDL_RenderPresent(renderTarget);

		aPoints.clear();
		bPoints.clear();
		vPoints.clear();

		sumOfFPS += FPS;
		numFrames++;
	}

	std::cout << "Average FPS: " << (sumOfFPS / numFrames) << '\n';

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
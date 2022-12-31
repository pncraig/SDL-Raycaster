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
#include "Ray.h"
#include "vec2.h"

// Size of the screen which the raycast scene is projected to (doesn't include the map)
const int width = 1280;
const int height = 800;

int projectionPlaneCenter{ height / 2 };	// The vertical center of the projection plane

bool DEBUG{ false };	// Set equal to true for an overhead view of the scene

// Stores the distance to the scene at each column
float zBuffer[width];

const int gridSize{ 64 };		// Side length of an individual grid block
const int gridWidth{ 20 };		// Width of the whole map in terms of grid blocks
const int gridHeight{ 20 };		// Height of the whole map in terms of grid blocks
std::string gridMap{};	// String which stores the map

Texture* wallTextureLocations[gridWidth * gridHeight];
Texture* floorTextureLocations[gridWidth * gridHeight];
Texture* ceilingTextureLocations[gridWidth * gridHeight];

// Vector which holds all the sprites
std::vector<Sprite> sprites{};

int FOV{ 60 };							// Field of view of player
int distanceToProjectionPlane{ 512 };	// Distance of the "camera" (player) to the "projection plane" (screen)

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
float jumpTime{ 0.0f };

// Used to calculate the time elapsed between frames
float previousTime{};
float currentTime{};
float deltaTime{};
float FPS{};

std::vector<vec2> points(width);	// Holds the intersection points that are used in rendering
std::vector<vec2> floorPoints{};		// Points where the floor texture is sampled

// In RGBA format
enum Color
{
	RED = 0xFF0000FF,
	BLUE = 0x0000FFFF,
	GREEN = 0x00FF00FF,
	PURPLE = 0xFF00FFFF,
	YELLOW = 0xFFFF00FF,
	TEAL = 0x00FFFFFF,
	WHITE = 0xFFFFFFFF,
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
	sprites.push_back(Sprite{ &spriteTexture, gridSize * 5.5f, gridSize * 17.5f });
	sprites.push_back(Sprite{ &spriteTexture, 350.0f + 127.0f, 350.0f });
	sprites.push_back(Sprite{ &spriteTexture2, 11.5f * gridSize, 13.5f * gridSize });
	sprites.push_back(Sprite{ &spriteTexture2, 18.5f * gridSize, 13.5f * gridSize });
	sprites.push_back(Sprite{ {&front, &right, &back, &left}, 16.0f * gridSize, 5.0f * gridSize, 0.0f });

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

				floorTextureLocations[location] = &mossy;
				ceilingTextureLocations[location] = &wood;
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
			screen[i] = 0;

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
			// theta + FOV * 0.5 is the angle of the first ray at the far left
			// FOV / width is the angle between the rays
			// * x is to determine the angle of the current ray
			// float rayAngle{ (theta + FOV * 0.5f) - (static_cast<float>(FOV) / width) * static_cast<float>(x) };

			// Calculate the angle between two rays
			float angleBetween{ degrees(atanf(static_cast<float>(x - (width / 2)) / adjustedDistanceToProjectionPlane)) };

			// Find the angle of the ray
			float rayAngle{ theta - angleBetween };

			Ray ray{ playerX, playerY, rayAngle };

			vec2 intersectionPoint{ ray.cast(gridMap, gridWidth, gridHeight, gridSize) };
			points[x] = intersectionPoint;

			// Determine the smaller distance
			float distance{ sqrtf((intersectionPoint.x - playerX) * (intersectionPoint.x - playerX) + (intersectionPoint.y - playerY) * (intersectionPoint.y - playerY)) };

			int gridIntersectionX{ static_cast<int>(intersectionPoint.x / gridSize) * gridSize };
			int gridIntersectionY{ static_cast<int>(intersectionPoint.y / gridSize) * gridSize };

			Texture* wallTexture{ wallTextureLocations[(gridIntersectionY / gridSize) * gridWidth + (gridIntersectionX / gridSize)] };

			// The column the ray hits on a wall
			int gridSpaceColumn{};

			// If the ray hits the left side of the wall...
			if (static_cast<int>(intersectionPoint.x) == gridIntersectionX)
			{
				// First column on the left side of the wall; at the top left corner of the wall
				gridSpaceColumn = static_cast<int>(intersectionPoint.y) - gridIntersectionY;
			}

			// right side
			if (static_cast<int>(intersectionPoint.x) == gridIntersectionX + gridSize - 1)
			{
				// First column on the right side of the wall; at the bottom right corner of the wall
				gridSpaceColumn = (gridIntersectionY + gridSize - 1) - static_cast<int>(intersectionPoint.y);
			}

			// top side
			if (static_cast<int>(intersectionPoint.y) == gridIntersectionY)
			{
				// First column on the top of the wall; at the top left corner of the wall
				gridSpaceColumn = (gridIntersectionX + gridSize - 1) - static_cast<int>(intersectionPoint.x);
			}

			// bottom side
			if (static_cast<int>(intersectionPoint.y) == gridIntersectionY + gridSize - 1)
			{
				// First column on the bottom of the wall; at the bottom right corner of the wall
				gridSpaceColumn = static_cast<int>(intersectionPoint.x) - gridIntersectionX;
			}

			// Calculate the lighting each wall sliver experiences, if the player were a light
			float lighting = -0.4 * distance + 255;

			// If the light level is less than 0, clamp to zero
			if (lighting < 0)
				lighting = 0;


			// Correct fish-eye distortion for the actual rendering of the walls
			distance *= cosf(radians(theta - rayAngle));

			// I use the fisheye corrected distance because the comparison in the sprite rendering loop uses the fisheye corrected distance
			// to the sprite
			zBuffer[x] = distance;

			// Calculate the height of the wall
			int wallHeight{ static_cast<int>((distanceToProjectionPlane * gridSize) / distance) };
			int halfWallHeight{ static_cast<int>(wallHeight / 2.0f) };

			// Y-coordinates of the bottom and top of the wall. Calculated in terms of player height and projection plane center (using similar
			// triangles) so that when the player height changes, the location of the wall will as well
			int bottomOfWall{ static_cast<int>(projectionPlaneCenter + (distanceToProjectionPlane * playerHeight) / distance) };
			int topOfWall{ bottomOfWall - wallHeight };

			// Calculate the gap between the top of the wall and the top of the screen; this is used for drawing the wall sliver
			// int ceilingGap{ static_cast<int>((height - wallHeight) / 2) };

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

				// screen[y * width + x] = calculateLighting(color, lighting);
				screen[y * width + x] = color;
			}

			// Precalculate some values that will be used in the for loop below
			float cosOfRayAngle{ cosf(radians(rayAngle)) };
			float sinOfRayAngle{ sinf(radians(rayAngle)) };
			float cosOfThetaMinusRayAngle{ cosf(radians(theta - rayAngle)) };
			
			Texture* floorTexture{};
			
			// Floor cast
			// y is a point on the projection plane from the bottom of the wall to the end of the screen
			for (int y{ bottomOfWall }; y < height; y++)
			{
				// The straight, vertical line distance to the point on the floor
				float straightDistance{ static_cast<float>(playerHeight * distanceToProjectionPlane) / (y - projectionPlaneCenter) };

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

				if (DEBUG)
					floorPoints.push_back(vec2{ pX, pY });

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

				// Calculate the lighting at that point on the floor
				float lighting{ -0.4f * correctedDistance + 255.0f };

				if (lighting < 0.0f)
					lighting = 0.0f;

				// screen[y * width + x] = calculateLighting(floorTexture[textureY * floorTexture.m_width + textureX], lighting);
				screen[y * width + x] = (*floorTexture)[textureY * floorTexture->m_width + textureX];
			}
			
			Texture* ceilingTexture{};

			// Ceiling casting. Basically the same process as floorcasting, except from the top of the wall up
			for (int y{ topOfWall }; y > 0; y--)
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

			// Draw the rays which are used to render the scene
			SDL_SetRenderDrawColor(renderTarget, 255, 0, 0, 0);
			for (int i{ 0 }; i < points.size(); i++)
			{
				float normPointX{ points[i].x / (gridSize * gridWidth) * height + width };
				float normPointY{ points[i].y / (gridSize * gridHeight) * height };
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
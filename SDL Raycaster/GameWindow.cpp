#include "GameWindow.h"

GameWindow::GameWindow()
	: width{ 640 },
	height{ 400 },
	cellSize{ 64 },
	maxWallHeight{ 128 },
	mapWidth{ 10 },
	mapHeight{ 20 },
	infiniteFloor{ false }
{
	screen = new uint32_t[static_cast<long int>(width) * height];
	wallHeights = new int[static_cast<long int>(mapWidth) * mapHeight];
	
	fov = 60;
	projectionPlaneCenter = height / 2;
	floatProjectionPlaneCenter = static_cast<double>(projectionPlaneCenter);
	distanceToProjectionPlane = 512;
	adjustedDistanceToProjectionPlane = (width / 2) / abs(tan((fov / 2) * (M_PI / 180.0f)));

	playerX = 128.0;
	playerY = 128.0;
	playerA = 0.0;
	playerHeight = 0;
	floatPlayerHeight = static_cast<double>(playerHeight);
	playerSpeed = 100.0;
	playerTurnSpeed = 75.0;
	playerLookVerticalSpeed = 575.0;
	playerVerticalSpeed = 175.0;

	moveForward = false;
	moveBackward = false;
	moveUp = false;
	moveDown = false;
	lookLeft = false;
	lookRight = false;
	lookUp = false;
	lookDown = false;

	sumOfFps = 0.0;
	numFrames = 0;

	isGameRunning = true;
	win = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
	renderTarget = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	frameBuffer = SDL_CreateTexture(renderTarget, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);

	initSDL();

	loadMap();

	loadFloorTexture();
	loadWallTexture();
	loadCeilingTexture();
}

GameWindow::~GameWindow()
{
	delete[] wallHeights;
	delete[] screen;

	SDL_DestroyWindow(win);
	SDL_DestroyRenderer(renderTarget);
	SDL_DestroyTexture(frameBuffer);

	SDL_Quit();
	IMG_Quit();
	TTF_Quit();
	Mix_Quit();
}

// Loading functions
void GameWindow::loadMap()
{
	int mx{ maxWallHeight };
	std::vector<int> tempMap{
		mx, mx, mx, mx, mx, mx, mx, mx, mx, mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,  mx,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	mx,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	mx,	0,	0,	0,	0,	0,	0,	mx,
		mx, 0,	0,	54,	0,	0,	0,	0,	0,	mx,
		mx, 0,	0,	0,	44,	0,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	34,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	0,	24,	0,	0,	mx,
		mx, 0,	0,	0,	0,	32,	32,	32,	0,	mx,
		mx, 0,	0,	0,	0,	32,	0,	32,	0,	mx,
		mx, 0,  32, 16,	0,	32,	32,	32,	0,	mx,
		mx, 0,  48,	32,	0,	0,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	0,	0,	0,	0,	mx,
		mx, mx, mx, mx, mx, mx, mx, mx, mx, mx,
	};

	for (int i{ 0 }; i < mapWidth * mapHeight; i++)
	{
		wallHeights[i] = tempMap[i];
	}
}

void GameWindow::loadFloorTexture()
{
	floorTexture = { "Textures.png", {128 * 20, 0, 128, 128}, SDL_PIXELFORMAT_RGBA8888 };
	// floorTexture = { "mossy.png", SDL_PIXELFORMAT_RGBA8888 };
}

void GameWindow::loadWallTexture()
{
	wallTexture = { "Textures.png", {128 * 46, 0, 128, 128}, SDL_PIXELFORMAT_RGBA8888 };
	// wallTexture = { "eagle.png", SDL_PIXELFORMAT_RGBA8888 };
}

void GameWindow::loadCeilingTexture()
{
	ceilingTexture = { "Textures.png", {128 * 32, 0, 128, 128}, SDL_PIXELFORMAT_RGBA8888 };
	// ceilingTexture = { "greystone.png", SDL_PIXELFORMAT_RGBA8888 };
}

void GameWindow::initSDL()
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
}

// Functions that help run the game and get input from user
void GameWindow::runSDLEventLoop()
{
	// Event loop
	while (SDL_PollEvent(&ev) != 0)
	{
		switch (ev.type)
		{
		// Check if the exit button has been clicked
		case SDL_QUIT:
			isGameRunning = false;
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

	keyState = SDL_GetKeyboardState(NULL);
}

void GameWindow::printScreenToWindow()
{
	// Update the frame buffer texture with the pixel information from screen. This texture will then be output to the screen
	SDL_UpdateTexture(frameBuffer, NULL, screen, width * sizeof(uint32_t));

	// Clear the screen
	SDL_RenderClear(renderTarget);
	
	// Copy the rendered scene to the screen
	SDL_RenderCopy(renderTarget, frameBuffer, NULL, NULL);

	SDL_RenderPresent(renderTarget);
}

void GameWindow::update()
{
	previousTime = currentTime;
	currentTime = SDL_GetTicks() / 1000.0;
	deltaTime = currentTime - previousTime;
	fps = 1.0 / deltaTime;

	std::cout << "FPS = " << fps << ", Avg. FPS = " << sumOfFps / numFrames << "\n";
	std::cout << "Projection plane center = " << floatProjectionPlaneCenter << " bubba" << '\n';
	std::cout << "\x1b[2F";

	for (int i{ 0 }; i < width * height; i++)
		screen[i] = 0x00000000;

	keysPressed();

	// Calculate the separate speed components of the player
	double xSpeed{};
	double ySpeed{};

	// ySpeed is negative because we're using the typical degree layout (unit circle), but y is increasing downward in out coordinate grid
	//	     90				 ----------> x
	//       |				|
	//180 ---+--- 0			|
	//		 |				v
	//		270				y
	// Because sine is positive in the first two quadrants on the unit circle, but that direction is down in our coordinate grid,
	// we need to make it negative to flip the sign

	// Move the player forwards
	if (moveForward)
	{
		xSpeed = playerSpeed * cos(util::radians(playerA));
		ySpeed = -playerSpeed * sin(util::radians(playerA));
	}

	// Move the player backwards
	if (moveBackward)
	{
		xSpeed = -playerSpeed * cos(util::radians(playerA));
		ySpeed = playerSpeed * sin(util::radians(playerA));
	}

	playerX += xSpeed * deltaTime;
	playerY += ySpeed * deltaTime;

	// Move the player up and down
	if (moveUp)
		floatPlayerHeight += playerVerticalSpeed * deltaTime;
	else if (moveDown)
		floatPlayerHeight -= playerVerticalSpeed * deltaTime;

	// Turn player left and right
	if (lookLeft)
		playerA += playerTurnSpeed * deltaTime;
	else if (lookRight)
		playerA -= playerTurnSpeed * deltaTime;

	// Move player view up and down
	if (lookUp)
		floatProjectionPlaneCenter += playerLookVerticalSpeed * deltaTime;
	else if (lookDown)
		floatProjectionPlaneCenter -= playerLookVerticalSpeed * deltaTime;

	// Keep the center of the projection plane from going off the screen
	if (floatProjectionPlaneCenter <= -height)
		floatProjectionPlaneCenter = -height;
	else if (floatProjectionPlaneCenter >= 2.0 * height)
		floatProjectionPlaneCenter = static_cast<double>(2.0 * height);

	// Make sure the player doesn't fly above or below the world
	if (floatPlayerHeight >= maxWallHeight)
		floatPlayerHeight = static_cast<double>(maxWallHeight - 1);
	else if (floatPlayerHeight <= 1.0)
		floatPlayerHeight = 1.0;

	playerHeight = static_cast<int>(floatPlayerHeight);
	projectionPlaneCenter = static_cast<int>(floatProjectionPlaneCenter);

	drawFullCeiling();
	raycast();

	sumOfFps += fps;
	numFrames++;
}

void GameWindow::keysPressed()
{
	moveForward = keyState[SDL_SCANCODE_W];
	moveBackward = keyState[SDL_SCANCODE_S];
	moveUp = keyState[SDL_SCANCODE_SPACE];
	moveDown = keyState[SDL_SCANCODE_LSHIFT];
	lookLeft = keyState[SDL_SCANCODE_A];
	lookRight = keyState[SDL_SCANCODE_D];
	lookUp = keyState[SDL_SCANCODE_UP];
	lookDown = keyState[SDL_SCANCODE_DOWN];
}

void GameWindow::run()
{
	while (isGameRunning)
	{
		runSDLEventLoop();
		update();
		printScreenToWindow();
	}
}

// Rendering functions functions
void GameWindow::drawFullFloor()
{
	// Precalculate some values that will be used in the loop below
	double cosOfPlayerAMinusHalfFOV{ cos(util::radians(playerA - fov * 0.5)) };
	double sinOfPlayerAMinusHalfFOV{ sin(util::radians(playerA - fov * 0.5)) };
	
	double cosOfPlayerAPlusHalfFOV{ cos(util::radians(playerA + fov * 0.5)) };
	double sinOfPlayerAPlusHalfFOV{ sin(util::radians(playerA + fov * 0.5)) };
	
	double cosOfHalfFOV{ cos(util::radians(fov * 0.5)) };

	// Draw Floor
	// Loop over every horizontal line from the center of the projection plane down. Exclude the line at the center because
	// it would cause a divide by 0 error
	for (int y{ std::max(projectionPlaneCenter + 1, 0) }; y < height; y++)
	{
		// Calculate how far the strip of floor corresponding to the horizontal line on the projection plane is from the player
		double distanceToStripe{ (distanceToProjectionPlane * static_cast<double>(playerHeight)) / (y - projectionPlaneCenter) };

		// Calculate the corrected distance (the walls are calculated with the corrected distance, so the floor must be as well)
		double correctedDistance{ distanceToStripe / cosOfHalfFOV };

		// Calculate the point on the horizontal stripe to the far left of the FOV
		double adX{ correctedDistance * cosOfPlayerAMinusHalfFOV };
		double adY{ correctedDistance * -sinOfPlayerAMinusHalfFOV };

		double aX{ playerX + adX };
		double aY{ playerY + adY };

		// Calculate the point on the horizontal stripe to the far right of the FOV
		double bdX{ correctedDistance * cosOfPlayerAPlusHalfFOV };
		double bdY{ correctedDistance * -sinOfPlayerAPlusHalfFOV };

		double bX{ playerX + bdX };
		double bY{ playerY + bdY };

		// Calculate a vector that points from point A to point B
		double vX{ bX - aX };
		double vY{ bY - aY };

		// Divide the vector by the width of the screen so that each time the vector is added to point A, a new pixel 
		// on the screen is represented
		double floorStepX{ vX / width };
		double floorStepY{ vY / width };

		// A for loop that goes accross the horizontal line
		for (int x{ width - 1 }; x >= 0; x--)
		{
			// If the point where the floor is to be sampled is outside of the map, move to the next point
			if (!infiniteFloor && (aX < 0.0f || aX > mapWidth * cellSize || aY < 0.0f || aY > mapHeight * cellSize))
			{
				aX += floorStepX;
				aY += floorStepY;
				continue;
			}

			// Calculate the coordinates of the grid square the point on the floor is in
			int gridX{ static_cast<int>(abs(aX) / cellSize) * cellSize };
			int gridY{ static_cast<int>(abs(aY) / cellSize) * cellSize };

			// Calculate the normalized coordinates of the point in terms of the grid square it is in
			double normX{ (abs(aX) - gridX) / static_cast<double>(cellSize) };
			double normY{ (abs(aY) - gridY) / static_cast<double>(cellSize) };

			if (aX < 0.0)
				normX = 1.0 - normX;

			if (aY < 0.0)
				normY = 1.0 - normY;

			// Calculate where those normalized coordinates fall on the texture of the floor
			int textureX{ static_cast<int>(normX * floorTexture.width()) };
			int textureY{ static_cast<int>(normY * floorTexture.height()) };

			// Write the color to the screen array
			screen[y * width + x] = floorTexture.getTexel(textureX, textureY);

			// Increment point A to get the next point on the floor
			aX += floorStepX;
			aY += floorStepY;
		}
	}
	
}

void GameWindow::drawFullCeiling()
{
	// Precalculate some values that will be used in the loop below
	double cosOfPlayerAMinusHalfFOV{ cos(util::radians(playerA - fov * 0.5)) };
	double sinOfPlayerAMinusHalfFOV{ sin(util::radians(playerA - fov * 0.5)) };

	double cosOfPlayerAPlusHalfFOV{ cos(util::radians(playerA + fov * 0.5)) };
	double sinOfPlayerAPlusHalfFOV{ sin(util::radians(playerA + fov * 0.5)) };

	double cosOfHalfFOV{ cos(util::radians(fov * 0.5)) };

	for (int y{ std::min(projectionPlaneCenter - 1, height - 1) }; y >= 0; y--)
	{
		// Calculate how far the strip of ceiling corresponding to the horizontal line on the projection plane is from the player
		double distanceToStripe{ (distanceToProjectionPlane * static_cast<double>(maxWallHeight - playerHeight)) / (projectionPlaneCenter - y) };

		// Calculate the corrected distance (the walls are calculated with the corrected distance, so the ceiling must be as well)
		double correctedDistance{ distanceToStripe / cosOfHalfFOV };

		// Calculate the point on the horizontal stripe to the far left of the FOV
		double adX{ correctedDistance * cosOfPlayerAMinusHalfFOV };
		double adY{ correctedDistance * -sinOfPlayerAMinusHalfFOV };

		double aX{ playerX + adX };
		double aY{ playerY + adY };

		// Calculate the point on the horizontal stripe to the far right of the FOV
		double bdX{ correctedDistance * cosOfPlayerAPlusHalfFOV };
		double bdY{ correctedDistance * -sinOfPlayerAPlusHalfFOV };
		
		double bX{ playerX + bdX };
		double bY{ playerY + bdY };

		// Calculate a vector that points from point A to point B
		double vX{ bX - aX };
		double vY{ bY - aY };

		// Divide the vector by the width of the screen so that each time the vector is added to point A, a new pixel 
		// on the screen is represented
		double floorStepX{ vX / width };
		double floorStepY{ vY / width };

		// A for loop that goes accross the horizontal line
		for (int x{ width - 1 }; x >= 0; x--)
		{
			if (!infiniteFloor && (aX < 0.0 || aX >= mapWidth * cellSize || aY < 0.0 || aY >= mapHeight * cellSize))
			{
				aX += floorStepX;
				aY += floorStepY;
				continue;
			}

			// Calculate the coordinates of the grid square the point on the ceiling is in
			int gridX{ static_cast<int>(abs(aX) / cellSize) * cellSize };
			int gridY{ static_cast<int>(abs(aY) / cellSize) * cellSize };

			// Calculate the normalized coordinates of the point in terms of the grid square it is in
			double normX{ (abs(aX) - gridX) / static_cast<double>(cellSize) };
			double normY{ (abs(aY) - gridY) / static_cast<double>(cellSize) };

			if (aX < 0.0)
				normX = 1.0 - normX;

			if (aY < 0.0)
				normY = 1.0 - normY;

			// Calculate where those normalized coordinates fall on the texture of the ceiling
			int textureX{ static_cast<int>(normX * ceilingTexture.width()) };
			int textureY{ static_cast<int>(normY * ceilingTexture.height()) };

			// Write the color to the screen array
			screen[y * width + x] = ceilingTexture.getTexel(textureX, textureY);

			// Increment point A to get the next point on the floor
			aX += floorStepX;
			aY += floorStepY;
		}
	}
}

void GameWindow::raycast()
{
	// Send a ray out into the scene for each vertical row of pixels in the screen array
	for (int x{ 0 }; x < width; x++)
	{
		// Calculate the angle between two rays
		double angleBetween{ util::degrees(atan(static_cast<double>(x - (width / 2)) / adjustedDistanceToProjectionPlane)) };

		// Find the angle of the ray
		double rayAngle{ playerA - angleBetween };

		// Precalculate some values that will be used in the floor and ceiling casting loops below
		double cosOfRayAngle{ cos(util::radians(rayAngle)) };
		double sinOfRayAngle{ sin(util::radians(rayAngle))};
		double cosOfThetaMinusRayAngle{ cos(util::radians(playerA - rayAngle)) };

		int previousTopOfWall{ height };	// Saves the position of the top of the previous wall on the projection plane to draw the floor on top of that wall

		bool finished{ false };

		double light{ 255.0 };

		// Distance from the player to the current intersection with the grid (this version of the raycasters checks every intersection 
		// between the ray and the grid)
		double distance{ 0.0 };

		// A number on the interval 0 < gridSpaceColumn < gridSize. The column on the grid where the ray intersects the wall
		int gridSpaceColumn{};

		// Precalculate a value that will be used more times
		double tanOfRayAngle{ tan(util::radians(rayAngle)) };

		// Find the angle of the ray on the interval 0 <= rayAngle < 360
		rayAngle = util::getCoterminalAngle(rayAngle);

		if (rayAngle == 360.0f)
			rayAngle = 0.0f;

		// These two boolean values are used to determine how to texture the wall by determining which side of the wall the ray hit
		bool topOrBottom{};	// True means the ray hit the top of the wall, false means it hit the bottom
		bool leftOrRight{};	// True means the ray hit the left of the wall, false means it hit the right

		// Point I is the intersection point (chosen between A and B)
		double iX{};
		double iY{};

		// CALCULATE HORIZONTAL INTERSECTIONS

		// Distance to intersection with horizontal grid line
		double horizontalDistance{ -1.0 };

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
			topOrBottom = false;

			// The first intersection will be part of the grid below (calculates y-coordinate of grid line below)
			aY = floor(playerY / static_cast<double>(cellSize)) * cellSize;

			// The next intersection with a horizontal grid line will be gridSize units below
			adY = -static_cast<double>(cellSize);

			//	      90					90		
			//  -x,-y | +x,-y			-tan | +tan
			// 180 ---+--- 0		  180 ---+--- 0	
			//	-x,+y |	+x,+y			+tan | -tan
			//		 270				    270		
			// When rayAngle < 90, dx should be >0, and when rayAngle > 90, dx should be <0
			// It just so happens that tan is >0 when rayAngle < 90 degrees, and tan is <0 when rayAngle > 90
			// so I don't have to change the signs at all
			adX = cellSize / tanOfRayAngle;

			// Calculate the x-coordinate of the first intersection with a horizontal gridline
			aX = playerX - (aY - playerY) / tanOfRayAngle;

			// Make part of the grid below for ease of checking for a wall
			aY -= 0.001;
		}
		// If ray is facing down... (or upwards on the coordinate grid)
		else
		{
			// The ray is facing down, so the ray hits the top of the wall
			topOrBottom = true;

			// The first horizontal grid intersection is with the grid line above the player
			aY = floor(playerY / static_cast<double>(cellSize)) * cellSize + cellSize;

			// The next gridline with be gridSize units above the player
			adY = static_cast<double>(cellSize);

			//	      90					90		
			//  -x,-y | +x,-y			-tan | +tan
			// 180 ---+--- 0		  180 ---+--- 0	
			//	-x,+y |	+x,+y			+tan | -tan
			//		 270				    270		
			// When rayAngle < 270, dx should be <0, and when rayAngle > 270, dx should be >0
			// It just so happens that tan is >0 when rayAngle < 270 degrees, and tan is <0 when rayAngle > 270
			// so I have to flip the signs with the negative
			adX = -cellSize / tanOfRayAngle;

			// Calculate the x-coordinate of the first intersection with a horizontal gridline
			aX = playerX - (aY - playerY) / tanOfRayAngle;
		}

		// CALCULATE VERTICAL INTERSECTIONS (very similar to calculating horizontal intersections)

		// Distance to intersection with vertical grid line
		double verticalDistance{ -1.0 };

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
			leftOrRight = true;

			// The first intersection will be in a grid to the right of the current grid
			bX = floor(playerX / cellSize) * cellSize + cellSize;

			// The ray is moving in a positive x-direction
			bdX = static_cast<double>(cellSize);

			//	      90					90		
			//  -x,-y | +x,-y			-tan | +tan
			// 180 ---+--- 0		  180 ---+--- 0	
			//	-x,+y |	+x,+y			+tan | -tan
			//		 270				    270		
			// When rayAngle < 180, dy should be <0, and when rayAngle > 180, dy should be >0
			// It just so happens that tan is >0 when rayAngle < 180 degrees, and tan is <0 when rayAngle > 180
			// so I have to flip the signs with the negative
			bdY = -tanOfRayAngle * cellSize;

			// Calculate the y-coordinate of the first intersection with a vertical gridline
			bY = playerY + (playerX - bX) * tanOfRayAngle;
		}
		// If the ray is facing to the left...
		else
		{
			// The ray is facing left so it will hit the wall to the right
			leftOrRight = false;

			// The first intersection will be in a grid to the left
			bX = floor(playerX / cellSize) * cellSize;

			// The ray is moving in a negative x-direction
			bdX = -static_cast<double>(cellSize);

			//	      90					90		
			//  -x,-y | +x,-y			-tan | +tan
			// 180 ---+--- 0		  180 ---+--- 0	
			//	-x,+y |	+x,+y			+tan | -tan
			//		 270				    270		
			// When rayAngle < 180, dy should be <0, and when rayAngle > 180, dy should be >0
			// It just so happens that tan is <0 when rayAngle < 180 degrees, and tan is >0 when rayAngle > 180
			// so I don't have to change the signs at all
			bdY = tanOfRayAngle * cellSize;

			// Calculate the y-coordinate of the first intersection with a vertical gridline
			bY = playerY + (playerX - bX) * tanOfRayAngle;

			bX -= 0.001;
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

			light = 255.0;

			// If the ray hit the top of the wall...
			if (topOrBottom)
			{
				// First column on the top of the wall; at the top left corner of the wall
				int gridX{ static_cast<int>(iX / cellSize) * cellSize + (cellSize - 1) };
				gridSpaceColumn = gridX - static_cast<int>(iX);
			}
			// Otherwise it hit the bottom of the wall
			else
			{
				// First column on the bottom of the wall; at the bottom right corner of the wall
				int gridX{ static_cast<int>(iX / cellSize) * cellSize };
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

			light = 128.0;

			// If the ray hit the left side of the wall...
			if (leftOrRight)
			{
				// First column on the left side of the wall; at the top left corner of the wall
				int gridY{ static_cast<int>(iY / cellSize) * cellSize };
				gridSpaceColumn = static_cast<int>(iY) - gridY;
			}
			// Otherwise it hit the right side of the wall
			else
			{
				// First column on the right side of the wall; at the bottom right corner of the wall
				int gridY{ static_cast<int>(iY / cellSize) * cellSize + (cellSize - 1) };
				gridSpaceColumn = gridY - static_cast<int>(iY);
			}
		}

		// The next 30 lines of code draw the walls of the grid the player is standing in; without these lines of code,
		// when the player stepped into a square with a wall in it, the walls would disappear

		// Determine the grid square the player is standing in
		int playerGridX{ static_cast<int>(playerX / cellSize) };
		int playerGridY{ static_cast<int>(playerY / cellSize) };

		// Determine the height of the floor beneath the player
		int heightBeneathPlayer{ wallHeights[playerGridY * mapWidth + playerGridX] };

		// Calculate the fisheye corrected distance used for rendering. All the variables below are prefaced by player
		// to distinguish them from the variables in the main rendering loop, not because they represent an attribute of the player
		double playerRenderingDistance{ distance * cos(util::radians(playerA - rayAngle)) };

		// Calculate the wall height in the player's grid square
		int playerWallHeight{ static_cast<int>(ceil((distanceToProjectionPlane / playerRenderingDistance) * heightBeneathPlayer)) };

		// Calculate the top and the bottom of the wall
		int playerBottomOfWall{ static_cast<int>(projectionPlaneCenter + (distanceToProjectionPlane * playerHeight) / playerRenderingDistance) };
		int playerTopOfWall{ playerBottomOfWall - playerWallHeight };

		Texture* playerWallTexture{ &wallTexture };
		Texture* playerFloorTexture{ &floorTexture };

		drawWallSlice(x, playerBottomOfWall, playerTopOfWall, gridSpaceColumn, playerWallHeight, light, playerWallTexture);

		if (playerHeight > heightBeneathPlayer)
		{
			drawVerticalFloorSlice(x, height - 1, playerTopOfWall, heightBeneathPlayer, rayAngle, playerFloorTexture);
		}
		else
		{
			drawVerticalCeilingSlice(x, playerTopOfWall, 0, heightBeneathPlayer, rayAngle, playerFloorTexture);
			
			drawVerticalFloorSlice(x, height - 1, playerBottomOfWall, 1, rayAngle, playerFloorTexture);

			finished = true;
		}

		// Update previousTopOfWall
		previousTopOfWall = playerTopOfWall;

		// Find all intersections between the ray and the grid, and render the wall of the given height
		while (!finished)
		{
			// Calculate which grid square the intersection belongs to
			int intersectionGridX{ static_cast<int>(iX / cellSize) };
			int intersectionGridY{ static_cast<int>(iY / cellSize) };

			Texture* wallTextur{ &wallTexture };
			Texture* floorTextur{ &floorTexture };

			// Look up the height of the wall at those grid coordinates
			int variableHeight{ wallHeights[intersectionGridY * mapWidth + intersectionGridX] };

			// Correct fish-eye distortion for the actual rendering of the walls
			double renderingDistance{ distance * cos(util::radians(playerA - rayAngle)) };

			// I use the fisheye corrected distance because the comparison in the sprite rendering loop uses the fisheye corrected distance
			// to the sprite
			// zBuffer[x] = distance;

			// The code up until the update of previousTopOfWall renders the front faces of the walls

			// Calculate the height of the wall
			int wallHeight{ static_cast<int>(ceil((distanceToProjectionPlane / renderingDistance) * variableHeight)) };

			// Y-coordinates of the bottom and top of the wall. Calculated in terms of player height and projection plane center (using similar
			// triangles) so that when the player height changes, the location of the wall will as well
			int bottomOfWall{ static_cast<int>(projectionPlaneCenter + (distanceToProjectionPlane * playerHeight) / renderingDistance) };
			int topOfWall{ bottomOfWall - wallHeight };

			if (bottomOfWall > previousTopOfWall)
				bottomOfWall = previousTopOfWall;

			drawWallSlice(x, bottomOfWall, topOfWall, gridSpaceColumn, wallHeight, light, wallTextur);

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

				light = 255.0;

				// If the ray hit the top of the wall...
				if (topOrBottom)
				{
					// First column on the top of the wall; at the top left corner of the wall
					int gridX{ static_cast<int>(iX / cellSize) * cellSize + (cellSize - 1) };
					gridSpaceColumn = gridX - static_cast<int>(iX);
				}
				// Otherwise it hit the bottom of the wall
				else
				{
					// First column on the bottom of the wall; at the bottom right corner of the wall
					int gridX{ static_cast<int>(iX / cellSize) * cellSize };
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

				light = 128.0;

				// If the ray hit the left side of the wall...
				if (leftOrRight)
				{
					// First column on the left side of the wall; at the top left corner of the wall
					int gridY{ static_cast<int>(iY / cellSize) * cellSize };
					gridSpaceColumn = static_cast<int>(iY) - gridY;
				}
				// Otherwise it hit the right side of the wall
				else
				{
					// First column on the right side of the wall; at the bottom right corner of the wall
					int gridY{ static_cast<int>(iY / cellSize) * cellSize + (cellSize - 1) };
					gridSpaceColumn = gridY - static_cast<int>(iY);
				}
			}

			// Code up until after the for loop projects and renders the back faces of the wall. This process uses the same
			// methods as rendering the front faces, but it is done with the updated intersection and distance information.
			// However, instead of using the next grid square, it uses the same grid square that was used to render the front faces.

			double backRenderingDistance{ distance * cos(util::radians(playerA - rayAngle)) };

			int backWallHeight{ static_cast<int>(ceil((distanceToProjectionPlane / backRenderingDistance) * variableHeight)) };

			int backBottomOfWall{ static_cast<int>(projectionPlaneCenter + (distanceToProjectionPlane * playerHeight) / backRenderingDistance) };
			int backTopOfWall{ backBottomOfWall - backWallHeight };

			if (backBottomOfWall > previousTopOfWall)
				backBottomOfWall = previousTopOfWall;

			// It is not necessary to render the back faces when the floors are being rendered
			/*
			
			drawWallSlice(x, backBottomOfWall, backTopOfWall, gridSpaceColumn, light, wallTextur);

			*/

			// Clip the values of topOfWall and backTopOfWall to the screen
			int minBetweenFrontTopOfWallAndHeight{ std::min(topOfWall, height - 1) };
			int maxBetweenBackTopOfWallAnd0{ std::max(backTopOfWall, 0) };

			// If the y-value at which the floor rendering starts is greater than the previous top of wall, then
			// that floor is obstructed and the y-value at which the floor rendering starts much be changed so
			// that it now starts at the top of the previous wall
			if (minBetweenFrontTopOfWallAndHeight > previousTopOfWall)
				minBetweenFrontTopOfWallAndHeight = previousTopOfWall;

			if (variableHeight != maxWallHeight)
			{
				drawVerticalFloorSlice(x, minBetweenFrontTopOfWallAndHeight, maxBetweenBackTopOfWallAnd0, variableHeight, rayAngle, floorTextur);
			}

			previousTopOfWall = std::min(previousTopOfWall, backTopOfWall);

			// If the height of the wall is the max height, then there is no need to continue finding 
			// wall intersections because this wall obstructs them from view. If the top of the wall is less
			// than 0, then the wall goes over the top of the screen. If the intersection point is outside the map,
			// then the raycasting process is finished
			if (iX < 0.0 || iX > cellSize * mapWidth || iY < 0.0 || iY > cellSize * mapHeight
				|| variableHeight == maxWallHeight || topOfWall < 0)
			{
				finished = true;
			}
		}
	}
}

void GameWindow::drawVerticalFloorSlice(int x, int startY, int endY, int floorHeight, double angle, Texture* texture)
{
	startY = util::clamp(startY, 0.0, static_cast<double>(height - 1));
	endY = util::clamp(endY, 0.0, static_cast<double>(height - 1));

	for (int y{ startY }; y >= endY; y--)
	{
		// Get the distance from the screen pixel to the point on the floor that it contains
		double floorDistance{ static_cast<double>((playerHeight - floorHeight) * distanceToProjectionPlane) / (y - projectionPlaneCenter) };

		// Correct for the fish eye effect
		floorDistance /= cos(util::radians(playerA - angle));

		// Calculate the point on the floor that contains the color of the pixel on the screen
		double floorX{ playerX + floorDistance * cos(util::radians(angle)) };
		double floorY{ playerY + floorDistance * -sin(util::radians(angle)) };

		// Keep the point within the bounds of the map to avoid accessing the floor texture in an improper way
		floorX = util::clamp(floorX, 0.0, mapWidth * cellSize);
		floorY = util::clamp(floorY, 0.0, mapHeight * cellSize);

		// Calculate the grid square the floor is in
		int floorGridX{ static_cast<int>(floorX / cellSize) * cellSize };
		int floorGridY{ static_cast<int>(floorY / cellSize) * cellSize };

		// Calculate the texture coordinates that correspond to the point on the floor
		double normX{ (floorX - floorGridX) / static_cast<double>(cellSize) };
		double normY{ (floorY - floorGridY) / static_cast<double>(cellSize) };

		int textureX{ static_cast<int>(normX * texture->width()) };
		int textureY{ static_cast<int>(normY * texture->height()) };

		int i{ textureY * texture->width() + textureX };
		uint32_t color{};

		if (i < texture->width() * texture->height())
			color = (*texture)[i];
		else
			color = 0x00FFFF00;

		screen[y * width + x] = color;
	}
}

void GameWindow::drawVerticalCeilingSlice(int x, int startY, int endY, int ceilingHeight, double angle, Texture* texture)
{
	startY = util::clamp(startY, 0.0, static_cast<double>(height - 1));
	endY = util::clamp(endY, 0.0, static_cast<double>(height - 1));

	for (int y{ std::min(startY, height - 1) }; y >= endY; y--)
	{
		// Get the distance from the screen pixel to the point on the floor that it contains
		double floorDistance{ static_cast<double>((ceilingHeight - playerHeight) * distanceToProjectionPlane) / (projectionPlaneCenter - y) };

		// Correct for the fish eye effect
		floorDistance /= cos(util::radians(playerA- angle));

		// Calculate the point on the floor that contains the color of the pixel on the screen
		double floorX{ playerX + floorDistance * cos(util::radians(angle)) };
		double floorY{ playerY + floorDistance * -sin(util::radians(angle)) };

		// Keep the point within the bounds of the map to avoid accessing the floor texture in an improper way
		floorX = util::clamp(floorX, 0.0, mapWidth * cellSize);
		floorY = util::clamp(floorY, 0.0f, mapHeight * cellSize);

		// Calculate the grid square the floor is in
		int floorGridX{ static_cast<int>(floorX / cellSize) * cellSize };
		int floorGridY{ static_cast<int>(floorY / cellSize) * cellSize };

		// Calculate the texture coordinates that correspond to the point on the floor
		double normX{ (floorX - floorGridX) / static_cast<double>(cellSize) };
		double normY{ (floorY - floorGridY) / static_cast<double>(cellSize) };

		int textureX{ static_cast<int>(normX * texture->width()) };
		int textureY{ static_cast<int>(normY * texture->height()) };

		screen[y * width + x] = texture->getTexel(textureX, textureY);
	}
}

void GameWindow::drawWallSlice(int x, int startY, int endY, int cellSpaceColumn, int wallHeight, double light, Texture* texture)
{
	// Calculate the texture column for the wall slice
	int textureSpaceColumn{ static_cast<int>(static_cast<double>(cellSpaceColumn) / cellSize * texture->width()) };

	int minBetweenHeightAndStartY{ std::min(height, startY) };

	// Draw the wall sliver
	for (int y{ std::max(endY, 0) }; y < minBetweenHeightAndStartY; y++)
	{
		// The row on the texture
		int textureSpaceRow{ static_cast<int>((y - endY) / static_cast<double>(wallHeight) * texture->height()) };

		// Get the color of the texture at the point on the wall (x, y)
		uint32_t color{ texture->getTexel(textureSpaceColumn, textureSpaceRow) };

		screen[y * width + x] = util::calculateLighting(color, light);
		// screen[y * width + x] = color;
	}
}
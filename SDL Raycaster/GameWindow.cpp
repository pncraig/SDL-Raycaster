#include "GameWindow.h"

GameWindow::GameWindow()
	: WIDTH{ 640 },
	HEIGHT{ 400 },
	CELL_SIZE{ 64 },
	MAX_WALL_HEIGHT{ 128 },
	MAP_WIDTH{ 10 },
	MAP_HEIGHT{ 60 },
	INFINITE_FLOOR{ false }
{
	SCREEN = new uint32_t[static_cast<long int>(WIDTH) * HEIGHT];
	WALL_HEIGHTS = new int[static_cast<long int>(MAP_WIDTH) * MAP_HEIGHT];
	
	FOV = 90;
	PROJECTION_PLANE_CENTER = HEIGHT / 2;
	FLOAT_PROJECTION_PLANE_CENTER = static_cast<double>(PROJECTION_PLANE_CENTER);
	DISTANCE_TO_PROJECTION_PLANE = (WIDTH / 2) / abs(tan(util::radians(FOV / 2)));

	PLAYER_X = 156.0 * 2;
	PLAYER_Y = 135.0 * 2;
	PLAYER_A = -90.0;
	PLAYER_HEIGHT = MAX_WALL_HEIGHT / 2;
	FLOAT_PLAYER_HEIGHT = static_cast<double>(PLAYER_HEIGHT);
	PLAYER_SPEED = 100.0;
	PLAYER_TURN_SPEED = 75.0;
	PLAYER_LOOK_VERTICAL_SPEED = 575.0;
	PLAYER_VERTICAL_SPEED = 175.0;

	PLAYER.setX(156.0 * 2);
	PLAYER.setY(134.0 * 2);
	PLAYER.setZ(MAX_WALL_HEIGHT / 2.0);
	PLAYER.setA(-90.0);

	MOVE_FORWARD = false;
	MOVE_BACKWARD = false;
	MOVE_UP = false;
	MOVE_DOWN = false;
	LOOK_LEFT = false;
	LOOK_RIGHT = false;
	LOOK_UP = false;
	LOOK_DOWN = false;
	ESCAPE = false;

	SUM_OF_FPS = 0.0;
	NUM_FRAMES = 0;

	IS_GAME_RUNNING = true;
	WIN = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
	RENDER_TARGET = SDL_CreateRenderer(WIN, -1, SDL_RENDERER_ACCELERATED);
	FRAME_BUFFER = SDL_CreateTexture(RENDER_TARGET, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

	initSDL();

	loadMap();

	loadFloorTexture();
	loadWallTexture();
	loadCeilingTexture();
}

GameWindow::~GameWindow()
{
	delete[] WALL_HEIGHTS;
	delete[] SCREEN;

	SDL_DestroyWindow(WIN);
	SDL_DestroyRenderer(RENDER_TARGET);
	SDL_DestroyTexture(FRAME_BUFFER);

	SDL_Quit();
	IMG_Quit();
	TTF_Quit();
	Mix_Quit();
}

// Loading functions
void GameWindow::loadMap()
{
	int mx{ MAX_WALL_HEIGHT };
	std::vector<int> tempMap{
		mx, mx, mx, mx, mx, mx, mx, mx, mx, mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,  mx,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	mx,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	0,
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
		mx, 0,  0,  0,  0,  0,  0,  0,  0,  mx,
		mx, 0,  0,  0,  0,  0,  0,  0,  0,  mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,  mx,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	mx,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	0,
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
		mx, 0,  0,  0,  0,  0,  0,  0,  0,  mx,
		mx, 0,  0,  0,  0,  0,  0,  0,  0,  mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,  mx,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	mx,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	mx,
		mx, 0,	0,	0,	0,	mx,	0,	0,	0,	0,
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

	for (int i{ 0 }; i < MAP_WIDTH * MAP_HEIGHT; i++)
	{
		WALL_HEIGHTS[i] = tempMap[i];
	}
}

void GameWindow::loadFloorTexture()
{
	FLOOR_TEXTURE = { "Textures.png", {128 * 20, 0, 128, 128}, SDL_PIXELFORMAT_RGBA8888 };
	// floorTexture = { "mossy.png", SDL_PIXELFORMAT_RGBA8888 }; 373694468
}

void GameWindow::loadWallTexture()
{
	WALL_TEXTURE = { "Textures.png", {128 * 47, 0, 128, 128}, SDL_PIXELFORMAT_RGBA8888 };
}

void GameWindow::loadCeilingTexture()
{
	CEILING_TEXTURE = { "Textures.png", {128 * 31, 0, 128, 128}, SDL_PIXELFORMAT_RGBA8888 };
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
	while (SDL_PollEvent(&EV) != 0)
	{
		switch (EV.type)
		{
		// Check if the exit button has been clicked
		case SDL_QUIT:
			IS_GAME_RUNNING = false;
			break;

		// Check for window events
		case SDL_WINDOWEVENT:
		// If the window is minimized, wait for events. Fixes memory spikes
			if (EV.window.event == SDL_WINDOWEVENT_MINIMIZED)
			{
				while (SDL_WaitEvent(&EV))
				{
					if (EV.window.event == SDL_WINDOWEVENT_RESTORED)
						break;
				}
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			mouseButtonDown();
			break;

		case SDL_MOUSEMOTION:
			mouseMoved();
			break;
		}
	}
	
	KEY_STATE = SDL_GetKeyboardState(NULL);
}

void GameWindow::printScreenToWindow()
{
	// Update the frame buffer texture with the pixel information from screen. This texture will then be output to the screen
	SDL_UpdateTexture(FRAME_BUFFER, NULL, SCREEN, WIDTH * sizeof(uint32_t));

	// Clear the screen
	SDL_RenderClear(RENDER_TARGET);
	
	// Copy the rendered scene to the screen
	SDL_RenderCopy(RENDER_TARGET, FRAME_BUFFER, NULL, NULL);

	SDL_RenderPresent(RENDER_TARGET);
}

void GameWindow::update()
{
	PLAYER_TURN_SPEED = 0.0;
	PLAYER_LOOK_VERTICAL_SPEED = 0.0;

	runSDLEventLoop();

	PREVIOUS_TIME = CURRENT_TIME;
	CURRENT_TIME = SDL_GetTicks() / 1000.0;
	DELTA_TIME = CURRENT_TIME - PREVIOUS_TIME;
	FPS = 1.0 / DELTA_TIME;

	SDL_SetWindowTitle(WIN, std::to_string(static_cast<int>(FPS)).c_str());

	keysPressed();

	if (ESCAPE)
		SDL_SetRelativeMouseMode(SDL_FALSE);

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
	if (MOVE_FORWARD)
	{
		xSpeed = PLAYER_SPEED * cos(util::radians(PLAYER_A));
		ySpeed = -PLAYER_SPEED * sin(util::radians(PLAYER_A));
	}

	// Move the player backwards
	if (MOVE_BACKWARD)
	{
		xSpeed = -PLAYER_SPEED * cos(util::radians(PLAYER_A));
		ySpeed = PLAYER_SPEED * sin(util::radians(PLAYER_A));
	}

	PLAYER_X += xSpeed * DELTA_TIME;
	PLAYER_Y += ySpeed * DELTA_TIME;

	// Move the player up and down
	if (MOVE_UP)
		FLOAT_PLAYER_HEIGHT += PLAYER_VERTICAL_SPEED * DELTA_TIME;
	else if (MOVE_DOWN)
		FLOAT_PLAYER_HEIGHT -= PLAYER_VERTICAL_SPEED * DELTA_TIME;

	// Turn player left and right
	if (LOOK_LEFT)
		PLAYER_A += PLAYER_TURN_SPEED * DELTA_TIME;
	else if (LOOK_RIGHT)
		PLAYER_A -= PLAYER_TURN_SPEED * DELTA_TIME;

	// Move player view up and down
	if (LOOK_UP)
		FLOAT_PROJECTION_PLANE_CENTER += PLAYER_LOOK_VERTICAL_SPEED * DELTA_TIME;
	else if (LOOK_DOWN)
		FLOAT_PROJECTION_PLANE_CENTER -= PLAYER_LOOK_VERTICAL_SPEED * DELTA_TIME;

	PLAYER_A -= PLAYER_TURN_SPEED * DELTA_TIME;
	FLOAT_PROJECTION_PLANE_CENTER -= PLAYER_LOOK_VERTICAL_SPEED * DELTA_TIME;

	// Keep the center of the projection plane from going off the screen
	if (FLOAT_PROJECTION_PLANE_CENTER <= -HEIGHT)
		FLOAT_PROJECTION_PLANE_CENTER = -HEIGHT;
	else if (FLOAT_PROJECTION_PLANE_CENTER >= 2.0 * HEIGHT)
		FLOAT_PROJECTION_PLANE_CENTER = static_cast<double>(2.0 * HEIGHT);

	// Make sure the player doesn't fly above or below the world
	if (FLOAT_PLAYER_HEIGHT >= MAX_WALL_HEIGHT)
		FLOAT_PLAYER_HEIGHT = static_cast<double>(MAX_WALL_HEIGHT - 1);
	else if (FLOAT_PLAYER_HEIGHT <= 1.0)
		FLOAT_PLAYER_HEIGHT = 1.0;

	PLAYER_HEIGHT = static_cast<int>(FLOAT_PLAYER_HEIGHT);
	PROJECTION_PLANE_CENTER = static_cast<int>(FLOAT_PROJECTION_PLANE_CENTER);

	SUM_OF_FPS += FPS;
	NUM_FRAMES++;
}

void GameWindow::render()
{

	drawFullCeiling();
	raycast();
}

void GameWindow::keysPressed()
{
	MOVE_FORWARD = KEY_STATE[SDL_SCANCODE_W];
	MOVE_BACKWARD = KEY_STATE[SDL_SCANCODE_S];
	MOVE_UP = KEY_STATE[SDL_SCANCODE_SPACE];
	MOVE_DOWN = KEY_STATE[SDL_SCANCODE_LSHIFT];
	LOOK_LEFT = KEY_STATE[SDL_SCANCODE_A];
	LOOK_RIGHT = KEY_STATE[SDL_SCANCODE_D];
	LOOK_UP = KEY_STATE[SDL_SCANCODE_UP];
	LOOK_DOWN = KEY_STATE[SDL_SCANCODE_DOWN];
	ESCAPE = KEY_STATE[SDL_SCANCODE_ESCAPE];
}

void GameWindow::run()
{
	while (IS_GAME_RUNNING)
	{
		update();
		render();
		printScreenToWindow();
	}
}

void GameWindow::mouseButtonDown()
{
	SDL_SetRelativeMouseMode(SDL_TRUE);
}

void GameWindow::mouseMoved()
{
	PLAYER_TURN_SPEED = EV.motion.xrel * 50.0;
	PLAYER_LOOK_VERTICAL_SPEED = EV.motion.yrel * 150.0;
}

// Rendering functions functions
void GameWindow::drawFullFloor()
{
	// Precalculate some values that will be used in the loop below
	double cosOfPlayerAMinusHalfFOV{ cos(util::radians(PLAYER_A - FOV * 0.5)) };
	double sinOfPlayerAMinusHalfFOV{ sin(util::radians(PLAYER_A - FOV * 0.5)) };
	
	double cosOfPlayerAPlusHalfFOV{ cos(util::radians(PLAYER_A + FOV * 0.5)) };
	double sinOfPlayerAPlusHalfFOV{ sin(util::radians(PLAYER_A + FOV * 0.5)) };
	
	double cosOfHalfFOV{ cos(util::radians(FOV * 0.5)) };

	// Draw Floor
	// Loop over every horizontal line from the center of the projection plane down. Exclude the line at the center because
	// it would cause a divide by 0 error
	for (int y{ std::max(PROJECTION_PLANE_CENTER + 1, 0) }; y < HEIGHT; y++)
	{
		// Calculate how far the strip of floor corresponding to the horizontal line on the projection plane is from the player
		double distanceToStripe{ (DISTANCE_TO_PROJECTION_PLANE * static_cast<double>(PLAYER_HEIGHT)) / (y - PROJECTION_PLANE_CENTER) };

		// Calculate the corrected distance (the walls are calculated with the corrected distance, so the floor must be as well)
		double correctedDistance{ distanceToStripe / cosOfHalfFOV };

		// Calculate the point on the horizontal stripe to the far left of the FOV
		double adX{ correctedDistance * cosOfPlayerAMinusHalfFOV };
		double adY{ correctedDistance * -sinOfPlayerAMinusHalfFOV };

		double aX{ PLAYER_X + adX };
		double aY{ PLAYER_Y + adY };

		// Calculate the point on the horizontal stripe to the far right of the FOV
		double bdX{ correctedDistance * cosOfPlayerAPlusHalfFOV };
		double bdY{ correctedDistance * -sinOfPlayerAPlusHalfFOV };

		double bX{ PLAYER_X + bdX };
		double bY{ PLAYER_Y + bdY };

		// Calculate a vector that points from point A to point B
		double vX{ bX - aX };
		double vY{ bY - aY };

		// Divide the vector by the width of the screen so that each time the vector is added to point A, a new pixel 
		// on the screen is represented
		double floorStepX{ vX / WIDTH };
		double floorStepY{ vY / WIDTH };

		// A for loop that goes accross the horizontal line
		for (int x{ WIDTH - 1 }; x >= 0; x--)
		{
			// If the point where the floor is to be sampled is outside of the map, move to the next point
			if (!INFINITE_FLOOR && (aX < 0.0f || aX > MAP_WIDTH * CELL_SIZE || aY < 0.0f || aY > MAP_HEIGHT * CELL_SIZE))
			{
				aX += floorStepX;
				aY += floorStepY;
				continue;
			}

			// Calculate the coordinates of the grid square the point on the floor is in
			int gridX{ static_cast<int>(abs(aX) / CELL_SIZE) * CELL_SIZE };
			int gridY{ static_cast<int>(abs(aY) / CELL_SIZE) * CELL_SIZE };

			// Calculate the normalized coordinates of the point in terms of the grid square it is in
			double normX{ (abs(aX) - gridX) / static_cast<double>(CELL_SIZE) };
			double normY{ (abs(aY) - gridY) / static_cast<double>(CELL_SIZE) };

			if (aX < 0.0)
				normX = 1.0 - normX;

			if (aY < 0.0)
				normY = 1.0 - normY;

			// Write the color to the screen array
			SCREEN[y * WIDTH + x] = FLOOR_TEXTURE.getTexel(normX, normY);

			// Increment point A to get the next point on the floor
			aX += floorStepX;
			aY += floorStepY;
		}
	}
	
}

void GameWindow::drawFullCeiling()
{
	// Precalculate some values that will be used in the loop below
	double cosOfPlayerAMinusHalfFOV{ cos(util::radians(PLAYER_A - FOV * 0.5)) };
	double sinOfPlayerAMinusHalfFOV{ sin(util::radians(PLAYER_A - FOV * 0.5)) };

	double cosOfPlayerAPlusHalfFOV{ cos(util::radians(PLAYER_A + FOV * 0.5)) };
	double sinOfPlayerAPlusHalfFOV{ sin(util::radians(PLAYER_A + FOV * 0.5)) };

	double cosOfHalfFOV{ cos(util::radians(FOV * 0.5)) };

	for (int y{ std::min(PROJECTION_PLANE_CENTER - 1, HEIGHT - 1) }; y >= 0; y--)
	{
		// Calculate how far the strip of ceiling corresponding to the horizontal line on the projection plane is from the player
		double distanceToStripe{ (DISTANCE_TO_PROJECTION_PLANE * static_cast<double>(MAX_WALL_HEIGHT - PLAYER_HEIGHT)) / (PROJECTION_PLANE_CENTER - y) };

		// Calculate the corrected distance (the walls are calculated with the corrected distance, so the ceiling must be as well)
		double correctedDistance{ distanceToStripe / cosOfHalfFOV };

		// Calculate the point on the horizontal stripe to the far left of the FOV
		double adX{ correctedDistance * cosOfPlayerAMinusHalfFOV };
		double adY{ correctedDistance * -sinOfPlayerAMinusHalfFOV };

		double aX{ PLAYER_X + adX };
		double aY{ PLAYER_Y + adY };

		// Calculate the point on the horizontal stripe to the far right of the FOV
		double bdX{ correctedDistance * cosOfPlayerAPlusHalfFOV };
		double bdY{ correctedDistance * -sinOfPlayerAPlusHalfFOV };
		
		double bX{ PLAYER_X + bdX };
		double bY{ PLAYER_Y + bdY };

		// Calculate a vector that points from point A to point B
		double vX{ bX - aX };
		double vY{ bY - aY };

		// Divide the vector by the width of the screen so that each time the vector is added to point A, a new pixel 
		// on the screen is represented
		double floorStepX{ vX / WIDTH };
		double floorStepY{ vY / WIDTH };

		// A for loop that goes accross the horizontal line
		for (int x{ WIDTH - 1 }; x >= 0; x--)
		{
			if (!INFINITE_FLOOR && (aX < 0.0 || aX >= MAP_WIDTH * CELL_SIZE || aY < 0.0 || aY >= MAP_HEIGHT * CELL_SIZE))
			{
				aX += floorStepX;
				aY += floorStepY;
				continue;
			}

			// Calculate the coordinates of the grid square the point on the ceiling is in
			int gridX{ static_cast<int>(abs(aX) / CELL_SIZE) * CELL_SIZE };
			int gridY{ static_cast<int>(abs(aY) / CELL_SIZE) * CELL_SIZE };

			// Calculate the normalized coordinates of the point in terms of the grid square it is in
			double normX{ (abs(aX) - gridX) / static_cast<double>(CELL_SIZE) };
			double normY{ (abs(aY) - gridY) / static_cast<double>(CELL_SIZE) };

			if (aX < 0.0)
				normX = 1.0 - normX;

			if (aY < 0.0)
				normY = 1.0 - normY;

			// Write the color to the screen array
			SCREEN[y * WIDTH + x] = CEILING_TEXTURE.getTexel(normX, normY);

			// Increment point A to get the next point on the floor
			aX += floorStepX;
			aY += floorStepY;
		}
	}
}

void GameWindow::raycast()
{
	// Send a ray out into the scene for each vertical row of pixels in the screen array
	for (int x{ 0 }; x < WIDTH; x++)
	{
		// Calculate the angle between two rays
		double angleBetween{ util::degrees(atan(static_cast<double>(x - (WIDTH / 2)) / DISTANCE_TO_PROJECTION_PLANE)) };

		// Find the angle of the ray
		double rayAngle{ PLAYER_A - angleBetween };

		// Precalculate some values that will be used in the floor and ceiling casting loops below
		double cosOfThetaMinusRayAngle{ cos(util::radians(PLAYER_A - rayAngle)) };

		int previousTopOfWall{ HEIGHT };	// Saves the position of the top of the previous wall on the projection plane to draw the floor on top of that wall

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
			aY = floor(PLAYER_Y / static_cast<double>(CELL_SIZE)) * CELL_SIZE;

			// The next intersection with a horizontal grid line will be gridSize units below
			adY = -static_cast<double>(CELL_SIZE);

			//	      90					90		
			//  -x,-y | +x,-y			-tan | +tan
			// 180 ---+--- 0		  180 ---+--- 0	
			//	-x,+y |	+x,+y			+tan | -tan
			//		 270				    270		
			// When rayAngle < 90, dx should be >0, and when rayAngle > 90, dx should be <0
			// It just so happens that tan is >0 when rayAngle < 90 degrees, and tan is <0 when rayAngle > 90
			// so I don't have to change the signs at all
			adX = CELL_SIZE / tanOfRayAngle;

			// Calculate the x-coordinate of the first intersection with a horizontal gridline
			aX = PLAYER_X - (aY - PLAYER_Y) / tanOfRayAngle;

			// Make part of the grid below for ease of checking for a wall
			aY -= 0.001;
		}
		// If ray is facing down... (or upwards on the coordinate grid)
		else
		{
			// The ray is facing down, so the ray hits the top of the wall
			topOrBottom = true;

			// The first horizontal grid intersection is with the grid line above the player
			aY = floor(PLAYER_Y / static_cast<double>(CELL_SIZE)) * CELL_SIZE + CELL_SIZE;

			// The next gridline with be gridSize units above the player
			adY = static_cast<double>(CELL_SIZE);

			//	      90					90		
			//  -x,-y | +x,-y			-tan | +tan
			// 180 ---+--- 0		  180 ---+--- 0	
			//	-x,+y |	+x,+y			+tan | -tan
			//		 270				    270		
			// When rayAngle < 270, dx should be <0, and when rayAngle > 270, dx should be >0
			// It just so happens that tan is >0 when rayAngle < 270 degrees, and tan is <0 when rayAngle > 270
			// so I have to flip the signs with the negative
			adX = -CELL_SIZE / tanOfRayAngle;

			// Calculate the x-coordinate of the first intersection with a horizontal gridline
			aX = PLAYER_X - (aY - PLAYER_Y) / tanOfRayAngle;
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
			bX = floor(PLAYER_X / CELL_SIZE) * CELL_SIZE + CELL_SIZE;

			// The ray is moving in a positive x-direction
			bdX = static_cast<double>(CELL_SIZE);

			//	      90					90		
			//  -x,-y | +x,-y			-tan | +tan
			// 180 ---+--- 0		  180 ---+--- 0	
			//	-x,+y |	+x,+y			+tan | -tan
			//		 270				    270		
			// When rayAngle < 180, dy should be <0, and when rayAngle > 180, dy should be >0
			// It just so happens that tan is >0 when rayAngle < 180 degrees, and tan is <0 when rayAngle > 180
			// so I have to flip the signs with the negative
			bdY = -tanOfRayAngle * CELL_SIZE;

			// Calculate the y-coordinate of the first intersection with a vertical gridline
			bY = PLAYER_Y + (PLAYER_X - bX) * tanOfRayAngle;
		}
		// If the ray is facing to the left...
		else
		{
			// The ray is facing left so it will hit the wall to the right
			leftOrRight = false;

			// The first intersection will be in a grid to the left
			bX = floor(PLAYER_X / CELL_SIZE) * CELL_SIZE;

			// The ray is moving in a negative x-direction
			bdX = -static_cast<double>(CELL_SIZE);

			//	      90					90		
			//  -x,-y | +x,-y			-tan | +tan
			// 180 ---+--- 0		  180 ---+--- 0	
			//	-x,+y |	+x,+y			+tan | -tan
			//		 270				    270		
			// When rayAngle < 180, dy should be <0, and when rayAngle > 180, dy should be >0
			// It just so happens that tan is <0 when rayAngle < 180 degrees, and tan is >0 when rayAngle > 180
			// so I don't have to change the signs at all
			bdY = tanOfRayAngle * CELL_SIZE;

			// Calculate the y-coordinate of the first intersection with a vertical gridline
			bY = PLAYER_Y + (PLAYER_X - bX) * tanOfRayAngle;

			bX -= 0.001;
		}

		// Determine which initial point (point A or point B) is closer to the player
		horizontalDistance = (PLAYER_X - aX) * (PLAYER_X - aX) + (PLAYER_Y - aY) * (PLAYER_Y - aY);
		verticalDistance = (PLAYER_X - bX) * (PLAYER_X - bX) + (PLAYER_Y - bY) * (PLAYER_Y - bY);

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
				int gridX{ static_cast<int>(iX / CELL_SIZE) * CELL_SIZE + (CELL_SIZE - 1) };
				gridSpaceColumn = gridX - static_cast<int>(iX);
			}
			// Otherwise it hit the bottom of the wall
			else
			{
				// First column on the bottom of the wall; at the bottom right corner of the wall
				int gridX{ static_cast<int>(iX / CELL_SIZE) * CELL_SIZE };
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
				int gridY{ static_cast<int>(iY / CELL_SIZE) * CELL_SIZE };
				gridSpaceColumn = static_cast<int>(iY) - gridY;
			}
			// Otherwise it hit the right side of the wall
			else
			{
				// First column on the right side of the wall; at the bottom right corner of the wall
				int gridY{ static_cast<int>(iY / CELL_SIZE) * CELL_SIZE + (CELL_SIZE - 1) };
				gridSpaceColumn = gridY - static_cast<int>(iY);
			}
		}

		// The next 30 lines of code draw the walls of the grid the player is standing in; without these lines of code,
		// when the player stepped into a square with a wall in it, the walls would disappear

		// Determine the grid square the player is standing in
		int playerGridX{ static_cast<int>(PLAYER_X / CELL_SIZE) };
		int playerGridY{ static_cast<int>(PLAYER_Y / CELL_SIZE) };

		// Determine the height of the floor beneath the player
		int heightBeneathPlayer{ WALL_HEIGHTS[playerGridY * MAP_WIDTH + playerGridX] };

		// Calculate the fisheye corrected distance used for rendering. All the variables below are prefaced by player
		// to distinguish them from the variables in the main rendering loop, not because they represent an attribute of the player
		double playerRenderingDistance{ distance * cosOfThetaMinusRayAngle };

		// Calculate the wall height in the player's grid square
		int playerWallHeight{ static_cast<int>(ceil((DISTANCE_TO_PROJECTION_PLANE / playerRenderingDistance) * heightBeneathPlayer)) };

		// Calculate the top and the bottom of the wall
		int playerBottomOfWall{ static_cast<int>(PROJECTION_PLANE_CENTER + (DISTANCE_TO_PROJECTION_PLANE * PLAYER_HEIGHT) / playerRenderingDistance) };
		int playerTopOfWall{ playerBottomOfWall - playerWallHeight };

		Texture* playerWallTexture{ &WALL_TEXTURE };
		Texture* playerFloorTexture{ &FLOOR_TEXTURE };

		drawWallSlice(x, playerBottomOfWall, playerTopOfWall, playerWallHeight, gridSpaceColumn, light, playerWallTexture);

		if (PLAYER_HEIGHT > heightBeneathPlayer)
		{
			drawVerticalFloorSlice(x, HEIGHT - 1, playerTopOfWall, heightBeneathPlayer, rayAngle, playerFloorTexture);
		}
		else
		{
			drawVerticalCeilingSlice(x, playerTopOfWall, 0, heightBeneathPlayer, rayAngle, playerFloorTexture);
			
			drawVerticalFloorSlice(x, HEIGHT - 1, playerBottomOfWall, 1, rayAngle, playerFloorTexture);

			finished = true;
		}

		// Update previousTopOfWall
		previousTopOfWall = playerTopOfWall;

		// Find all intersections between the ray and the grid, and render the wall of the given height
		while (!finished)
		{
			// Calculate which grid square the intersection belongs to
			int intersectionGridX{ static_cast<int>(iX / CELL_SIZE) };
			int intersectionGridY{ static_cast<int>(iY / CELL_SIZE) };

			Texture* wallTexture{ &WALL_TEXTURE };
			Texture* floorTexture{ &FLOOR_TEXTURE };

			// Look up the height of the wall at those grid coordinates
			int variableHeight{ WALL_HEIGHTS[intersectionGridY * MAP_WIDTH + intersectionGridX] };

			// Correct fish-eye distortion for the actual rendering of the walls
			double renderingDistance{ distance * cosOfThetaMinusRayAngle };

			// I use the fisheye corrected distance because the comparison in the sprite rendering loop uses the fisheye corrected distance
			// to the sprite
			// zBuffer[x] = distance;

			// The code up until the update of previousTopOfWall renders the front faces of the walls

			// Calculate the height of the wall
			int wallHeight{ static_cast<int>(ceil((DISTANCE_TO_PROJECTION_PLANE / renderingDistance) * variableHeight)) };

			// Y-coordinates of the bottom and top of the wall. Calculated in terms of player height and projection plane center (using similar
			// triangles) so that when the player height changes, the location of the wall will as well
			int bottomOfWall{ static_cast<int>(PROJECTION_PLANE_CENTER + (DISTANCE_TO_PROJECTION_PLANE * PLAYER_HEIGHT) / renderingDistance) };
			int topOfWall{ bottomOfWall - wallHeight };

			if (bottomOfWall > previousTopOfWall)
				bottomOfWall = previousTopOfWall;

			drawWallSlice(x, bottomOfWall, topOfWall, wallHeight, gridSpaceColumn,  light, wallTexture);

			// Update previousTopOfWall, but only if the current wall is larger than the previous (a larger wall will have 
			// a smaller topOfWall variable value)
			previousTopOfWall = std::min(previousTopOfWall, topOfWall);

			// The code until backRenderingDistance is instantiated updates the grid intersection

			// Determine which initial point (point A or point B) is closer to the player
			horizontalDistance = (PLAYER_X - (aX + adX)) * (PLAYER_X - (aX + adX)) + (PLAYER_Y - (aY + adY)) * (PLAYER_Y - (aY + adY));
			verticalDistance = (PLAYER_X - (bX + bdX)) * (PLAYER_X - (bX + bdX)) + (PLAYER_Y - (bY + bdY)) * (PLAYER_Y - (bY + bdY));

			double prevIX{ iX };
			double prevIY{ iY };

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
					int gridX{ static_cast<int>(iX / CELL_SIZE) * CELL_SIZE + (CELL_SIZE - 1) };
					gridSpaceColumn = gridX - static_cast<int>(iX);
				}
				// Otherwise it hit the bottom of the wall
				else
				{
					// First column on the bottom of the wall; at the bottom right corner of the wall
					int gridX{ static_cast<int>(iX / CELL_SIZE) * CELL_SIZE };
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
					int gridY{ static_cast<int>(iY / CELL_SIZE) * CELL_SIZE };
					gridSpaceColumn = static_cast<int>(iY) - gridY;
				}
				// Otherwise it hit the right side of the wall
				else
				{
					// First column on the right side of the wall; at the bottom right corner of the wall
					int gridY{ static_cast<int>(iY / CELL_SIZE) * CELL_SIZE + (CELL_SIZE - 1) };
					gridSpaceColumn = gridY - static_cast<int>(iY);
				}
			}

			// Code up until after the for loop projects and renders the back faces of the wall. This process uses the same
			// methods as rendering the front faces, but it is done with the updated intersection and distance information.
			// However, instead of using the next grid square, it uses the same grid square that was used to render the front faces.

			double backRenderingDistance{ distance * cosOfThetaMinusRayAngle };

			int backWallHeight{ static_cast<int>(ceil((DISTANCE_TO_PROJECTION_PLANE / backRenderingDistance) * variableHeight)) };

			int backBottomOfWall{ static_cast<int>(PROJECTION_PLANE_CENTER + (DISTANCE_TO_PROJECTION_PLANE * PLAYER_HEIGHT) / backRenderingDistance) };
			int backTopOfWall{ backBottomOfWall - backWallHeight };

			if (backBottomOfWall > previousTopOfWall)
				backBottomOfWall = previousTopOfWall;

			// It is not necessary to render the back faces when the floors are being rendered
			/*
			
			drawWallSlice(x, backBottomOfWall, backTopOfWall, backWallHeight, gridSpaceColumn, light, wallTexture);

			*/

			// Clip the values of topOfWall and backTopOfWall to the screen
			int minBetweenFrontTopOfWallAndHeight{ std::min(topOfWall, HEIGHT - 1) };
			int maxBetweenBackTopOfWallAnd0{ std::max(backTopOfWall, 0) };

			// If the y-value at which the floor rendering starts is greater than the previous top of wall, then
			// that floor is obstructed and the y-value at which the floor rendering starts much be changed so
			// that it now starts at the top of the previous wall
			if (minBetweenFrontTopOfWallAndHeight > previousTopOfWall)
				minBetweenFrontTopOfWallAndHeight = previousTopOfWall;

			if (variableHeight != MAX_WALL_HEIGHT)
			{
				drawVerticalFloorSlice(x, minBetweenFrontTopOfWallAndHeight, maxBetweenBackTopOfWallAnd0, variableHeight, rayAngle, floorTexture);
			}

			previousTopOfWall = std::min(previousTopOfWall, backTopOfWall);

			// If the height of the wall is the max height, then there is no need to continue finding 
			// wall intersections because this wall obstructs them from view. If the top of the wall is less
			// than 0, then the wall goes over the top of the screen. If the intersection point is outside the map,
			// then the raycasting process is finished
			if (iX < 0.0 || iX > CELL_SIZE * MAP_WIDTH - 1 || iY < 0.0 || iY > CELL_SIZE * MAP_HEIGHT - 1
				|| variableHeight == MAX_WALL_HEIGHT || topOfWall < 0)
			{
				finished = true;
			}
		}
	}
}

void GameWindow::drawVerticalFloorSlice(int x, int startY, int endY, int floorHeight, double angle, Texture* texture)
{
	startY = util::clamp(startY, 0.0, static_cast<double>(HEIGHT - 1));
	endY = util::clamp(endY, 0.0, static_cast<double>(HEIGHT - 1));

	double cosOfPlayerAMinusAngle{ cos(util::radians(PLAYER_A - angle)) };
	double cosOfAngle{ cos(util::radians(angle)) };
	double sinOfAngle{ sin(util::radians(angle)) };

	for (int y{ startY }; y >= endY; y--)
	{
		// Get the distance from the screen pixel to the point on the floor that it contains
		double floorDistance{ static_cast<double>((PLAYER_HEIGHT - floorHeight) * DISTANCE_TO_PROJECTION_PLANE) / (y - PROJECTION_PLANE_CENTER) };

		// Correct for the fish eye effect
		floorDistance /= cosOfPlayerAMinusAngle;

		// Calculate the point on the floor that contains the color of the pixel on the screen
		double floorX{ PLAYER_X + floorDistance * cosOfAngle };
		double floorY{ PLAYER_Y + floorDistance * -sinOfAngle };

		// Keep the point within the bounds of the map to avoid accessing the floor texture in an improper way
		floorX = util::clamp(floorX, 0.0, MAP_WIDTH * CELL_SIZE);
		floorY = util::clamp(floorY, 0.0, MAP_HEIGHT * CELL_SIZE);

		// Calculate the grid square the floor is in
		int floorGridX{ static_cast<int>(floorX / CELL_SIZE) * CELL_SIZE };
		int floorGridY{ static_cast<int>(floorY / CELL_SIZE) * CELL_SIZE };

		// Calculate the texture coordinates that correspond to the point on the floor
		double normX{ (floorX - floorGridX) / static_cast<double>(CELL_SIZE) };
		double normY{ (floorY - floorGridY) / static_cast<double>(CELL_SIZE) };

		SCREEN[y * WIDTH + x] = texture->getTexel(normX, normY);
	}
}

void GameWindow::drawVerticalCeilingSlice(int x, int startY, int endY, int ceilingHeight, double angle, Texture* texture)
{
	startY = util::clamp(startY, 0.0, static_cast<double>(HEIGHT - 1));
	endY = util::clamp(endY, 0.0, static_cast<double>(HEIGHT - 1));

	double cosOfPlayerAMinusAngle{ cos(util::radians(PLAYER_A - angle)) };
	double cosOfAngle{ cos(util::radians(angle)) };
	double sinOfAngle{ sin(util::radians(angle)) };

	for (int y{ std::min(startY, HEIGHT - 1) }; y >= endY; y--)
	{
		// Get the distance from the screen pixel to the point on the floor that it contains
		double floorDistance{ static_cast<double>((ceilingHeight - PLAYER_HEIGHT) * DISTANCE_TO_PROJECTION_PLANE) / (PROJECTION_PLANE_CENTER - y) };

		// Correct for the fish eye effect
		floorDistance /= cosOfPlayerAMinusAngle;

		// Calculate the point on the floor that contains the color of the pixel on the screen
		double floorX{ PLAYER_X + floorDistance * cosOfAngle };
		double floorY{ PLAYER_Y + floorDistance * -sinOfAngle };

		// Keep the point within the bounds of the map to avoid accessing the floor texture in an improper way
		floorX = util::clamp(floorX, 0.0, MAP_WIDTH * CELL_SIZE);
		floorY = util::clamp(floorY, 0.0f, MAP_HEIGHT * CELL_SIZE);

		// Calculate the grid square the floor is in
		int floorGridX{ static_cast<int>(floorX / CELL_SIZE) * CELL_SIZE };
		int floorGridY{ static_cast<int>(floorY / CELL_SIZE) * CELL_SIZE };

		// Calculate the texture coordinates that correspond to the point on the floor
		double normX{ (floorX - floorGridX) / static_cast<double>(CELL_SIZE) };
		double normY{ (floorY - floorGridY) / static_cast<double>(CELL_SIZE) };

		SCREEN[y * WIDTH + x] = texture->getTexel(normX, normY);
	}
}

void GameWindow::drawWallSlice(int x, int startY, int endY, int wallHeight, int cellSpaceColumn, double light, Texture* texture)
{
	// Calculate the texture column for the wall slice
	int textureSpaceColumn{ static_cast<int>(static_cast<double>(cellSpaceColumn) / CELL_SIZE * texture->width()) };

	int minBetweenHeightAndStartY{ std::min(HEIGHT, startY) };

	// Draw the wall sliver
	for (int y{ std::max(endY, 0) }; y < minBetweenHeightAndStartY; y++)
	{
		// The row on the texture
		int textureSpaceRow{ static_cast<int>((y - endY) / static_cast<double>(wallHeight) * texture->height()) };

		// Get the color of the texture at the point on the wall (x, y)
		uint32_t color{ texture->getTexel(textureSpaceColumn, textureSpaceRow) };

		SCREEN[y * WIDTH + x] = util::calculateLighting(color, light);
		// screen[y * width + x] = color;
	}
}

void GameWindow::drawVerticalFloorSliceFast(int x, int startY, int endY, int sliceHeight, double aX, double aY, double bX, double bY, Texture* texture)
{
	double uX{ aX - static_cast<int>(aX / CELL_SIZE) * CELL_SIZE };
	double uY{ aY - static_cast<int>(aY / CELL_SIZE) * CELL_SIZE };

	double vX{ bX - static_cast<int>(bX / CELL_SIZE) * CELL_SIZE };
	double vY{ bY - static_cast<int>(bY / CELL_SIZE) * CELL_SIZE };

	double stepX{ (bX - aX) / (sliceHeight) };
	double stepY{ (bY - aY) / (sliceHeight) };

	for (int y{ startY }; y >= endY; y--)
	{
		double normX{ uX / texture->width() };
		double normY{ uY / texture->height() };

		uint32_t color{ texture->getTexel(normX, normY) };
		SCREEN[y * WIDTH + x] = color;

		uX += stepX;
		uY += stepY;
	}
}
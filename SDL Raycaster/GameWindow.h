#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include "Sprites.h"
#include "Texture.h"
#include "Utility.h"
#include "Player.h"
#include <string>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

class GameWindow
{
private:
	const int WIDTH{};		// Width of the screen in pixels
	const int HEIGHT{};		// Height of the screen in pixels

	const int CELL_SIZE{};			// Dimensions of one of the grid squares, or cells, on the map
	const int MAX_WALL_HEIGHT{};	// The maximum possible size of a wall
	int* WALL_HEIGHTS{};			// An array that contains the heights of each cell on the map

	const int MAP_WIDTH{};		// The width of the map in cells
	const int MAP_HEIGHT{};		// The height of the map in cells

	const bool INFINITE_FLOOR{};	// Should the floor and ceiling be drawn to the horizon or should just the parts of the floor on the map be drawn?

	Texture FLOOR_TEXTURE;		// Textures for the floor
	Texture WALL_TEXTURE;		// Textures for the walls
	Texture CEILING_TEXTURE;		// Textures for the ceiling

	int PROJECTION_PLANE_CENTER{};				// The center of the projection plane, which basically means the center of the screen
	double FLOAT_PROJECTION_PLANE_CENTER{};		// The center of the projection plane as a float so that the projection plane can move up and down based on fractional values
	double DISTANCE_TO_PROJECTION_PLANE{};			// The distance from the player's eyes to the projection plane, useful because it allows us to use similar triangles to calculate wall heights

	int FOV{};							// The angle with which the player views the scene; the larger the field of view, the larger the amount of scene that is visible
	double PLAYER_X{};					// X position of the player in units (NOT cell coordinates)
	double PLAYER_Y{};					// Y position of the player in units (NOT cell coordinates)
	double PLAYER_A{};					// Angle of the player in degrees
	int PLAYER_HEIGHT{};				// Height of the player's eyes
	double FLOAT_PLAYER_HEIGHT{};		// Player height as a float for the same reason as floatProjectionPlaneCenter
	double PLAYER_RADIUS{};				// Radius of the player in units (how much area the player takes up on the map
	double PLAYER_SPEED{};				// How fast the player moves around the map
	double PLAYER_TURN_SPEED{};			// How fast the view of the player shifts left and right
	double PLAYER_LOOK_VERTICAL_SPEED{};	// How fast the player can look up and down
	double PLAYER_VERTICAL_SPEED{};		// How fast the player can move up and down

	Player PLAYER{};

	bool MOVE_FORWARD{};		// Is the player moving forwards?
	bool MOVE_BACKWARD{};	// Is the player moving backwards?
	bool MOVE_UP{};			// Is the player moving up?
	bool MOVE_DOWN{};		// Is the player moving down?
	bool LOOK_LEFT{};	    // Is the player looking left?
	bool LOOK_RIGHT{};	    // Is the player looking right?
	bool LOOK_UP{};		    // Is the player looking up?
	bool LOOK_DOWN{};	    // Is the player looking down?
	bool ESCAPE{};

	double PREVIOUS_TIME{};
	double CURRENT_TIME{};
	double DELTA_TIME{};
	double FPS{};
	double SUM_OF_FPS{};
	int NUM_FRAMES{};

	uint32_t* SCREEN;		// Array of integers that contains the colors that will be output to the screen

	bool IS_GAME_RUNNING{};
	SDL_Window* WIN;
	SDL_Renderer* RENDER_TARGET;
	SDL_Event EV;
	SDL_Texture* FRAME_BUFFER;
	const Uint8* KEY_STATE;

public:
	GameWindow();		// Constructor for the GameWindow class
	~GameWindow();		// Deletes all the dynamic memory

	void loadMap();				// Load the map from somewhere (eventually from a file)
	void loadFloorTexture();	// Load the floor textures from somewhere (eventually from a file)
	void loadWallTexture();		// Load the wall textures from somewhere (eventually from a file)
	void loadCeilingTexture();	// Load the ceiling textures from somewhere (eventually from a file)
	void initSDL();

	void runSDLEventLoop();
	void printScreenToWindow();
	void update();
	void render();
	void keysPressed();
	void mouseButtonDown();
	void mouseMoved();
	void run();

	void drawFullFloor();
	void drawFullCeiling();
	void raycast();
	void drawVerticalFloorSlice(int x, int startY, int endY, int floorHeight, double angle, Texture* texture);
	void drawVerticalCeilingSlice(int x, int startY, int endY, int ceilingHeight, double angle, Texture* texture);
	void drawWallSlice(int x, int startY, int endY, int wallHeight, int cellSpaceColumn, double light, Texture* texture);
	void drawVerticalFloorSliceFast(int x, int startY, int endY, int sliceHeight, double uX, double uY, double vX, double vY, Texture* texture);
};
#endif


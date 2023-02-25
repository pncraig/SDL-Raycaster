#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include "Sprites.h"
#include "Texture.h"
#include "Utility.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

class GameWindow
{
private:
	const int width{};		// Width of the screen in pixels
	const int height{};		// Height of the screen in pixels

	const int cellSize{};		// Dimensions of one of the grid squares, or cells, on the map
	const int maxWallHeight{};	// The maximum possible size of a wall
	int* wallHeights{};			// An array that contains the heights of each cell on the map

	const int mapWidth{};		// The width of the map in cells
	const int mapHeight{};		// The height of the map in cells

	const bool infiniteFloor{};	// Should the floor and ceiling be drawn to the horizon or should just the parts of the floor on the map be drawn?

	Texture floorTexture;		// Textures for the floor
	Texture wallTexture;		// Textures for the walls
	Texture ceilingTexture;		// Textures for the ceiling

	int projectionPlaneCenter{};				// The center of the projection plane, which basically means the center of the screen
	double floatProjectionPlaneCenter{};		// The center of the projection plane as a float so that the projection plane can move up and down based on fractional values
	int distanceToProjectionPlane{};			// The distance from the player's eyes to the projection plane, useful because it allows us to use similar triangles to calculate wall heights
	double adjustedDistanceToProjectionPlane{};	// The "adjusted" distance to the projection plane, used so that I can adjust the field of view

	int fov{};							// The angle with which the player views the scene; the larger the field of view, the larger the amount of scene that is visible
	double playerX{};					// X position of the player in units (NOT cell coordinates)
	double playerY{};					// Y position of the player in units (NOT cell coordinates)
	double playerA{};					// Angle of the player in degrees
	int playerHeight{};					// Height of the player's eyes
	double floatPlayerHeight{};			// Player height as a float for the same reason as floatProjectionPlaneCenter
	double playerRadius{};				// Radius of the player in units (how much area the player takes up on the map
	double playerSpeed{};				// How fast the player moves around the map
	double playerTurnSpeed{};			// How fast the view of the player shifts left and right
	double playerLookVerticalSpeed{};	// How fast the player can look up and down
	double playerVerticalSpeed{};		// How fast the player can move up and down

	bool moveForward{};		// Is the player moving forwards?
	bool moveBackward{};	// Is the player moving backwards?
	bool moveUp{};			// Is the player moving up?
	bool moveDown{};		// Is the player moving down?
	bool lookLeft{};	    // Is the player looking left?
	bool lookRight{};	    // Is the player looking right?
	bool lookUp{};		    // Is the player looking up?
	bool lookDown{};	    // Is the player looking down?

	double previousTime{};
	double currentTime{};
	double deltaTime{};
	double fps{};
	double sumOfFps{};
	int numFrames{};

	uint32_t* screen;		// Array of integers that contains the colors that will be output to the screen

	bool isGameRunning{};
	SDL_Window* win;
	SDL_Renderer* renderTarget;
	SDL_Event ev;
	SDL_Texture* frameBuffer;
	const Uint8* keyState;

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
	void keysPressed();
	void run();

	void drawFullFloor();
	void drawFullCeiling();
	void raycast();
	void drawVerticalFloorSlice(int x, int startY, int endY, int floorHeight, double angle, Texture* texture);
	void drawVerticalCeilingSlice(int x, int startY, int endY, int ceilingHeight, double angle, Texture* texture);
	void drawWallSlice(int x, int startY, int endY, int cellSpaceColumn, int wallHeight, double light, Texture* texture);
};
#endif


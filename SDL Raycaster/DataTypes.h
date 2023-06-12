#pragma once

#include "SDL.h"
#include "Texture.h"

#define PLAYER_MOVE_FORWARD		SDL_SCANCODE_W
#define PLAYER_MOVE_BACKWARD	SDL_SCANCODE_S
#define PLAYER_ROTATE_LEFT		SDL_SCANCODE_A
#define PLAYER_ROTATE_RIGHT		SDL_SCANCODE_D

#define DISPLAY_MAP				SDL_SCANCODE_TAB

enum class side_t
{
	NORTH,		// Sides facing 90 degrees
	SOUTH,		// Sides facing 270 degrees
	EAST,		// Sides facing 0 degrees
	WEST,		// Sides facing 180 degrees
};

struct cell_t
{
	bool wall{};
	Texture* textures[4]{};
	Texture* floor{};
	Texture* ceiling{};
};

struct pointf_t
{
	double x{};
	double y{};
};
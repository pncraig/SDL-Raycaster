/*
* Created 2:41 PM on 12/25/2021 (Merry Christmas / Clashmas)
* 
*  To do:
*  - Create an optimized way for floorcasting in vertical strips. Also, change playerA to be in terms of radians and create a table for atan. In general, find optimizations
*  - Add sprites back in
*  - Add the ability to raycast without variable height walls
*  - Add back in the view being controlled by the mouse
*  - Add panoramic backgrounds, and do it the right way this time
*  - Add bilinear filtering back in or/and mipmaps
*  - Begin creating classes for things like the player and the screen so I can abstract some of the complexity away from the GameWindow class
*  - Add a physics system
*  - Add lighting and normal maps
*  - Add the ability to render variable height ceilings (and maybe the ability to do variable height floors at the same time)
*  - Add special walls that can be curved or at non 90 degree angles
*  - Add animated textures and sprites
*  - Find a more efficient manner of rendering
*/

#include "GameWindow.h"
#include "Game.h"

int main(int argc, char* argv[])
{
	/*GameWindow gameWindow{};

	gameWindow.run();*/

	Game game{};

	if(!game.init())
		return 1;
	
	while (game.isRunning())
	{
		game.handleInput();
		game.update();
		game.render();
	}

	return 0;
}
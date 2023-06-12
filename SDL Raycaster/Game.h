#pragma once

#include "RaycastEngine.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include <string>
#include "Settings.h"

class Game
{
private:
	RaycastEngine* m_GameEngine{};

	int m_WindowWidth{};
	int m_WindowHeight{};

	SDL_Window* m_Window{};
	SDL_Renderer* m_Renderer{};
	SDL_Texture* m_FrameBuffer{};

	double m_CurrentTime{};
	double m_PreviousTime{};
	double m_DeltaTime{};

public:
	Game();
	~Game();

	bool init();

	bool isRunning();

	void displayMap();

	void handleInput();

	void update();

	void render();
};


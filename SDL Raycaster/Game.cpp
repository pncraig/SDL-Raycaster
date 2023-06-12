#include "Game.h"

Game::Game()
	:m_WindowWidth{ WINDOW_WIDTH }, m_WindowHeight{ WINDOW_HEIGHT }
{
	m_GameEngine = new RaycastEngine();
}

Game::~Game()
{
	delete m_GameEngine;
	SDL_DestroyRenderer(m_Renderer);
	SDL_DestroyWindow(m_Window);
	SDL_DestroyTexture(m_FrameBuffer);
}

bool Game::init()
{
	// SDL_Init() returns a negative number upon failure, and SDL_INIT_EVERYTHING sets all the flags to true
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cout << "Error initializing SDL: " << SDL_GetError() << '\n';	// SDL_GetError() returns information about an error that occurred in a string
		return false;
	}

	// VVVVVVVVVVVVVVVVVVVVVVV Put image flags here
	int imgFlags{ IMG_INIT_PNG };
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		std::cout << "Error initializing IMG: " << IMG_GetError() << '\n';
		return false;
	}

	if (!m_GameEngine->init())
	{
		std::cout << "Failed to initialize game engine " << m_GameEngine->getName() << ".\n";
		return false;
	}

	m_Window = SDL_CreateWindow(m_GameEngine->getName().c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_WindowWidth, m_WindowHeight, 0);
	if (m_Window == NULL)
	{
		std::cout << "Failed to create a window. SDL Error: " << SDL_GetError() << "\n";
		return false;
	}

	m_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_ACCELERATED);
	if (m_Renderer == NULL)
	{
		std::cout << "Failed to create a renderer. SDL Error: " << SDL_GetError() << "\n";
		return false;
	}

	m_FrameBuffer = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, m_GameEngine->getRenderWidth(), m_GameEngine->getRenderHeight());
	if (m_FrameBuffer == NULL)
	{
		std::cout << "Failed to create a frame buffer. SDL Error: " << SDL_GetError() << '\n';
		return false;
	}

	if (SDL_RenderSetLogicalSize(m_Renderer, m_GameEngine->getRenderWidth(), m_GameEngine->getRenderHeight()) != 0)
	{
		std::cout << "Failed to set render logical size. SDL Error: " << SDL_GetError() << "\n";
		return false;
	}

	return true;
}

bool Game::isRunning() { return m_GameEngine->isGameRunning(); }

void Game::displayMap()
{
	Map* map{ m_GameEngine->getMap() };
	Player* player{ m_GameEngine->getPlayer() };

	int cellSize{};

	if (m_GameEngine->getRenderWidth() / map->getWidth() < m_GameEngine->getRenderHeight() / map->getHeight())
	{
		cellSize = m_GameEngine->getRenderWidth() / map->getWidth();
	}
	else
	{
		cellSize = m_GameEngine->getRenderHeight() / map->getHeight();
	}

	SDL_SetRenderDrawColor(m_Renderer, 255, 0, 255, 255);

	for (int y{ 0 }; y < map->getHeight(); y++)
	{
		for (int x{ 0 }; x < map->getWidth(); x++)
		{
			SDL_Rect r{ x * cellSize, y * cellSize, cellSize, cellSize };
			if (map->get(x, y).wall)
			{
				SDL_RenderFillRect(m_Renderer, &r);
			}
			else
			{
				SDL_RenderDrawRect(m_Renderer, &r);
			}
		}
	}

	int playerX{ static_cast<int>((player->getX() / (map->getWidth() * map->getCellSize())) * cellSize * map->getWidth()) };
	int playerY{ static_cast<int>((player->getY() / (map->getHeight() * map->getCellSize())) * cellSize * map->getHeight()) };

	SDL_SetRenderDrawColor(m_Renderer, 255, 0, 0, 255);

	for (int i{ 0 }; i < m_GameEngine->getRenderWidth(); i++)
	{
		double x{ m_GameEngine->getIntersections()[i].x };
		double y{ m_GameEngine->getIntersections()[i].y };

		int mappedX{ static_cast<int>((x / (map->getWidth() * map->getCellSize())) * cellSize * map->getWidth()) };
		int mappedY{ static_cast<int>((y / (map->getHeight() * map->getCellSize())) * cellSize * map->getHeight()) };

		SDL_RenderDrawLine(m_Renderer, playerX, playerY, mappedX, mappedY);
	}

	SDL_Rect p{ playerX - 2, playerY - 2, 4, 4 };

	SDL_SetRenderDrawColor(m_Renderer, 0, 0, 255, 255);
	SDL_RenderFillRect(m_Renderer, &p);
}

void Game::handleInput()
{
	SDL_Event event;
	// Event loop
	while (SDL_PollEvent(&event) != 0)
	{
		switch (event.type)
		{
			// Check if the exit button has been clicked
		case SDL_QUIT:
			m_GameEngine->quit();
			break;

			// Check for window events
		case SDL_WINDOWEVENT:
			// If the window is minimized, wait for events. Fixes memory spikes
			if (event.window.event == SDL_WINDOWEVENT_MINIMIZED)
			{
				while (SDL_WaitEvent(&event))
				{
					if (event.window.event == SDL_WINDOWEVENT_RESTORED)
						break;
				}
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			break;

		case SDL_MOUSEMOTION:
			break;
		}
	}

	const uint8_t* keystate = SDL_GetKeyboardState(NULL);
	m_GameEngine->handleKeys(keystate, m_DeltaTime);
}

void Game::update()
{
	// Calculate delta time
	m_PreviousTime = m_CurrentTime;
	m_CurrentTime = SDL_GetTicks() / 1000.0;
	m_DeltaTime = m_CurrentTime - m_PreviousTime;

	// Change the title of the window to include the frame rate
	int fps{ static_cast<int>(1.0 / m_DeltaTime) };
	std::string title{ m_GameEngine->getName() + " " + std::to_string(fps) };
	SDL_SetWindowTitle(m_Window, title.c_str());

	m_GameEngine->update(m_DeltaTime);

}

void Game::render()
{
	SDL_RenderClear(m_Renderer);

	m_GameEngine->render();

	SDL_UpdateTexture(m_FrameBuffer, NULL, m_GameEngine->getScreenPixels(), m_GameEngine->getRenderWidth() * sizeof(uint32_t));

	SDL_RenderCopy(m_Renderer, m_FrameBuffer, NULL, NULL);
	
	if(m_GameEngine->isMapDisplayed())
		displayMap();


	SDL_RenderPresent(m_Renderer);
}
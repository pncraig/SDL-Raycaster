#pragma once

#include "Player.h"
#include "Map.h"
#include "RaycastRenderer.h"
#include "Settings.h"
#include "DataTypes.h"

class RaycastEngine
{
private:
	Player* m_EnginePlayer{};
	Map* m_EngineMap{};
	RaycastRenderer* m_EngineRenderer{};

	bool m_EngineIsRunning{};
	bool m_DisplayMap{};

	int m_RenderWidth{};
	int m_RenderHeight{};

public:
	RaycastEngine();
	~RaycastEngine();

	bool init();

	int getRenderWidth();
	int getRenderHeight();

	std::string getName();

	void quit();
	bool isGameRunning();
	bool isMapDisplayed();

	uint32_t* getScreenPixels();
	pointf_t* getIntersections();
	Map* getMap();
	Player* getPlayer();

	void handleKeys(const uint8_t* keystate, double deltaTime);

	void update(double deltaTime);
	
	void render();
};


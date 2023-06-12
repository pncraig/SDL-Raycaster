#include "RaycastEngine.h"

RaycastEngine::RaycastEngine()
	:m_RenderWidth{ RENDER_WIDTH }, m_RenderHeight{ RENDER_HEIGHT }, m_EngineIsRunning{ true }, m_DisplayMap{ false }
{
	m_EngineMap = new Map();
	m_EnginePlayer = new Player(m_EngineMap->getCellSize() * 5, m_EngineMap->getCellSize() * 5, m_EngineMap->getMaxHeight() / 2, 0.0, 0);
	m_EngineRenderer = new RaycastRenderer(m_RenderWidth, m_RenderHeight, m_EngineMap, m_EnginePlayer);
}

RaycastEngine::~RaycastEngine()
{
	delete m_EnginePlayer;
	delete m_EngineMap;
	delete m_EngineRenderer;
}

bool RaycastEngine::init()
{
	if (!m_EngineRenderer->init())
	{
		std::cout << "Unable to initialize engine renderer\n";
		return false;
	}

	return true;
}

int RaycastEngine::getRenderWidth() { return m_RenderWidth; }
int RaycastEngine::getRenderHeight() { return m_RenderHeight; }

void RaycastEngine::quit() { m_EngineIsRunning = false; }
bool RaycastEngine::isGameRunning() { return m_EngineIsRunning; }
bool RaycastEngine::isMapDisplayed() { return m_DisplayMap; }

std::string RaycastEngine::getName() { return "Raycaster"; }

uint32_t* RaycastEngine::getScreenPixels() { return m_EngineRenderer->getScreenPixels(); }
pointf_t* RaycastEngine::getIntersections() { return m_EngineRenderer->getIntersections(); }

Map* RaycastEngine::getMap() { return m_EngineMap; }
Player* RaycastEngine::getPlayer() { return m_EnginePlayer; }

void RaycastEngine::handleKeys(const uint8_t* keystate, double deltaTime)
{
	if (keystate[PLAYER_MOVE_FORWARD])
		m_EnginePlayer->moveForward(deltaTime);
	else if (keystate[PLAYER_MOVE_BACKWARD])
		m_EnginePlayer->moveBackward(deltaTime);
	
	if (keystate[PLAYER_ROTATE_LEFT])
		m_EnginePlayer->rotateLeft(deltaTime);
	else if (keystate[PLAYER_ROTATE_RIGHT])
		m_EnginePlayer->rotateRight(deltaTime);

	if (keystate[DISPLAY_MAP])
		m_DisplayMap = true;
	else
		m_DisplayMap = false;
}

void RaycastEngine::update(double deltaTime)
{
	
}

void RaycastEngine::render()
{
	m_EngineRenderer->paintScreen(0x000000ff);

	m_EngineRenderer->renderFullFloor();
	m_EngineRenderer->renderFullCeiling();
	m_EngineRenderer->raycast();
}
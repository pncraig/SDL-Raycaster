#include "Map.h"

Map::Map()
{
	m_MapCellSize = 64;
	m_MapMaxWallHeight = 64;
	m_MapWidth = 10;
	m_MapHeight = 10;

	std::vector<bool> map
	{
		true, true, true, true, true, true, true, true, true, true,
		true, false, false, false, false, false, false, false, false, true,
		true, false, true, true, false, false, false, false, false, true,
		true, false, true, true, false, false, false, false, false, true,
		true, false, false, false, false, false, true, false, false, true,
		true, false, false, false, false, false, true, false, false, true,
		true, true, true, true, true, false, true, false, false, true,
		true, false, false, false, true, false, true, false, false, true,
		true, false, false, false, false, false, true, false, false, true,
		true, true, true, true, true, true, true, true, true, true,
	};

	Texture* a{ new Texture("eagle.png", 373694468) };
	Texture* b{ new Texture("greystone.png", 373694468) };
	Texture* c{ new Texture("bluestone.png", 373694468) };
	Texture* d{ new Texture("colorstone.png", 373694468) };
	Texture* e{ new Texture("mossy.png", 373694468) };
	Texture* f{ new Texture("purplestone.png", 373694468) };

	m_MapTextures.push_back(a);
	m_MapTextures.push_back(b);
	m_MapTextures.push_back(c);
	m_MapTextures.push_back(d);
	m_MapTextures.push_back(e);
	m_MapTextures.push_back(f);

	for (int i{ 0 }; i < 100; i++)
	{
		m_MapCells.push_back(cell_t{ map[i], { a, b, c, d }, e, f });
	}
}

Map::~Map()
{
	for (int i{ 0 }; i < m_MapTextures.size(); i++)
	{
		delete m_MapTextures[i];
		m_MapTextures[i] = nullptr;
	}
}

cell_t Map::get(int x, int y)
{
	return m_MapCells[y * m_MapWidth + x];
}

int Map::getWidth() { return m_MapWidth; }
int Map::getHeight() { return m_MapHeight; }

int Map::getCellSize() { return m_MapCellSize; }
int Map::getMaxHeight() { return m_MapMaxWallHeight; }
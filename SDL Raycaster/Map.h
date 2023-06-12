#pragma once
#include "DataTypes.h"
#include <iostream>
#include <vector>
#include "Texture.h"

class Map
{
private:
	std::vector<cell_t> m_MapCells{};
	std::vector<Texture*> m_MapTextures{};

	int m_MapWidth{};
	int m_MapHeight{};
	int m_MapMaxWallHeight{};
	int m_MapCellSize{};

public:
	Map();

	Map(std::string mapName);

	~Map();

	cell_t get(int x, int y);

	int getWidth();
	int getHeight();

	int getCellSize();
	int getMaxHeight();
};


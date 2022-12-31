#include "Scene.h"
#include <iostream>

Map::Map(int width, int height, std::string layout)
	: m_width{ width }, m_height{ height }, m_layout{ layout }
{

}

char Map::operator[](int i)
{
	if (i < 0 || i >= m_layout.size())
	{
		std::cout << "Out of bounds error in a map: " << i << '\n';
		return '\0';
	}
	else
		return m_layout[i];
}
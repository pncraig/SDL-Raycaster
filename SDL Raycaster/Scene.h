#ifndef SCENE_H
#define SCENE_H

#include <iostream>

class Map
{
private:
	int m_width;
	int m_height;
	std::string m_layout;
public:
	Map(int width, int height, std::string layout);
	
	int width()
	{
		return m_width;
	}

	int height()
	{
		return m_height;
	}

	char operator[] (int i);
};

#endif

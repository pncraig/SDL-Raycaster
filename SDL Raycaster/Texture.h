#ifndef TEXTURE_H
#define TEXTURE_H

#include "SDL.h"
#include <iostream>


class Texture
{
private:
	uint32_t* m_pixels{};

public:
	int m_width;
	int m_height;
	std::string m_fileName;

	Texture(const std::string& fileName, int pixelFormat);
	Texture(const std::string& fileName, const SDL_Rect& section, int pixelFormat);
	~Texture();

	uint32_t operator[](int i);
};

#endif


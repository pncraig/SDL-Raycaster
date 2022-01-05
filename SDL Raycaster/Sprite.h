#ifndef SPRITE_H
#define SPRITE_H

#include "Texture.h"
#include "SDL.h"

class Sprite
{
private:
	Texture m_texture;
public:
	float x{};
	float y{};

	Sprite(const std::string& fileName, int pixelFormat, float s_x, float s_y);

	uint32_t operator[](int i);
};

#endif

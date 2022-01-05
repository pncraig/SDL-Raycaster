#include "Sprite.h"
#include "Texture.h"

Sprite::Sprite(const std::string& fileName, int pixelFormat, float s_x, float s_y)
	: m_texture{ fileName, pixelFormat }, x{ s_x }, y{ s_y }
{
	
}

uint32_t Sprite::operator[](int i)
{
	return m_texture[i];
}
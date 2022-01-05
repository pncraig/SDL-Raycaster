#include "Sprite.h"
#include "Texture.h"

Sprite::Sprite(Texture* texture, float s_x, float s_y)
	: m_texture{ texture }, x{ s_x }, y{ s_y }
{
	m_textureWidth = m_texture->m_width;
	m_textureHeight = m_texture->m_height;
}


uint32_t Sprite::operator[](int i)
{
	return (*m_texture)[i];
}

int Sprite::textureWidth() { return m_textureWidth; }
int Sprite::textureHeight() { return m_textureHeight; }
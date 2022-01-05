#ifndef SPRITE_H
#define SPRITE_H

#include "Texture.h"
#include "SDL.h"

class Sprite
{
private:
	Texture* m_texture;
	int m_textureWidth{};
	int m_textureHeight{};
public:
	float x{};
	float y{};
	float noSqrtDistance{};		// Stores the distance calculated without the sqrt, so only useful for sorting

	Sprite() {}
	Sprite(Texture* texture, float s_x, float s_y);

	uint32_t operator[](int i);

	int textureWidth();
	int textureHeight();
};

#endif

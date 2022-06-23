#ifndef SPRITES_H
#define SPRITES_H

#include "Texture.h"
#include "SDL.h"
#include <vector>

class Sprite
{
private:
	std::vector<Texture*> m_textures;

	int m_angleBetween;
public:
	float x{};
	float y{};
	float theta{};				// Sprite angle
	float noSqrtDistance{};		// Stores the distance calculated without the sqrt, so only useful for sorting

	Sprite() {}
	Sprite(Texture* texture, float s_x, float s_y);
	Sprite(const std::vector<Texture*>& textures, float s_x, float s_y, float s_a);

	Texture* facingTexture(float a);
	float angleToSprite(float x, float y);
};

#endif

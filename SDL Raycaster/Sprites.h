#ifndef SPRITES_H
#define SPRITES_H

#include "Texture.h"
#include "SDL.h"
#include <vector>
#include "Utility.h"

class Sprite
{
private:
	std::vector<Texture*> m_textures;

	int m_angleBetween{};
public:
	double x{};
	double y{};
	double theta{};				// Sprite angle
	double noSqrtDistance{};		// Stores the distance calculated without the sqrt, so only useful for sorting

	Sprite() {}
	Sprite(Texture* texture, double s_x, double s_y);
	Sprite(const std::vector<Texture*>& textures, double s_x, double s_y, double s_a);

	Texture* facingTexture(double a);
	double angleToSprite(double x, double y);
};

#endif

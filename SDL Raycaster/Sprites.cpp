#include "Sprites.h"

Sprite::Sprite(Texture* texture, double s_x, double s_y)
	: x{ s_x }, y{ s_y }
{
	m_textures.push_back(texture);
	m_angleBetween = 360;
}

Sprite::Sprite(const std::vector<Texture*>& textures, double s_x, double s_y, double s_a)
	: m_textures{ textures }, x{ s_x }, y{ s_y }, theta{ s_a }
{
	m_angleBetween = 360 / static_cast<int>(textures.size());
}


Texture* Sprite::facingTexture(double a)
{
	theta = util::getCoterminalAngle(theta);

	double halfAngleBetween{ m_angleBetween * 0.5 };

	a += theta + halfAngleBetween;

	a = util::getCoterminalAngle(a);

	return m_textures[static_cast<int>(a / m_angleBetween)];
}

double Sprite::angleToSprite(double p_x, double p_y)
{
	return util::degrees(atan2(p_y - y, p_x - x));
}
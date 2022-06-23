#include "Sprites.h"
#include "Texture.h"

float getCoterminalAngle(float angle);
float degrees(float radians);

Sprite::Sprite(Texture* texture, float s_x, float s_y)
	: x{ s_x }, y{ s_y }
{
	m_textures.push_back(texture);
	m_angleBetween = 360;
}

Sprite::Sprite(const std::vector<Texture*>& textures, float s_x, float s_y, float s_a)
	: m_textures{ textures }, x{ s_x }, y{ s_y }, theta{ s_a }
{
	m_angleBetween = 360 / textures.size();
}


Texture* Sprite::facingTexture(float a)
{
	theta = getCoterminalAngle(theta);

	float halfAngleBetween{ m_angleBetween * 0.5f };

	a += theta + halfAngleBetween;

	a = getCoterminalAngle(a);

	return m_textures[static_cast<int>(a / m_angleBetween)];
}

float Sprite::angleToSprite(float p_x, float p_y)
{
	return degrees(atan2f(p_y - y, p_x - x));
}
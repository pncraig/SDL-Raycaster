#include "Texture.h"

Texture::Texture(const std::string& fileName, int pixelFormat)
{
	m_fileName = fileName;

	SDL_Surface* textureSurface{ IMG_Load(fileName.c_str()) };
	SDL_Surface* formattedSurface{ SDL_ConvertSurfaceFormat(textureSurface, pixelFormat, NULL) };

	m_width = formattedSurface->w;
	m_height = formattedSurface->h;
	m_pixels = new uint32_t[m_width * m_height];

	SDL_LockSurface(formattedSurface);

	uint32_t* pixelsToCopy = static_cast<uint32_t*>(formattedSurface->pixels);

	for (int i{ 0 }; i < m_width * m_height; i++)
		m_pixels[i] = pixelsToCopy[i];

	SDL_UnlockSurface(formattedSurface);

	SDL_FreeSurface(textureSurface);
	SDL_FreeSurface(formattedSurface);
}

Texture::Texture(const std::string& fileName, const SDL_Rect& section, int pixelFormat)
{
	m_fileName = fileName;

	SDL_Surface* textureSurface{ IMG_Load(fileName.c_str()) };
	SDL_Surface* formattedSurface{ SDL_ConvertSurfaceFormat(textureSurface, pixelFormat, NULL) };

	m_width = section.w;
	m_height = section.h;
	m_pixels = new uint32_t[m_width * m_height];

	if (m_width > formattedSurface->w)
		m_width = formattedSurface->w;

	if (m_height > formattedSurface->h)
		m_height = formattedSurface->h;

	SDL_LockSurface(formattedSurface);

	uint32_t* pixelsToCopy = static_cast<uint32_t*>(formattedSurface->pixels);

	for (int x{ section.x }; x < section.x + m_width; x++)
	{
		for (int y{ section.y }; y < section.y + m_height; y++)
		{
			m_pixels[(y - section.y) * m_width + (x - section.x)] = pixelsToCopy[y * formattedSurface->w + x];
		}
	}

	SDL_UnlockSurface(formattedSurface);

	SDL_FreeSurface(textureSurface);
	SDL_FreeSurface(formattedSurface);
}

Texture::Texture(const Texture& t)
{
	m_fileName = t.m_fileName;
	m_width = t.m_width;
	m_height = t.m_height;
	
	m_pixels = new uint32_t[m_width * m_height];

	for (int i{ 0 }; i < m_width * m_height; i++)
	{
		m_pixels[i] = t.m_pixels[i];
	}
}

Texture& Texture::operator=(const Texture& t)
{
	if (this == &t)
		return *this;

	m_fileName = t.m_fileName;
	m_width = t.m_width;
	m_height = t.m_height;

	m_pixels = new uint32_t[m_width * m_height];
	for (int i{ 0 }; i < m_width * m_height; i++)
		m_pixels[i] = t.m_pixels[i];

	return *this;
}

uint32_t Texture::operator[](int i)
{
	if (i >= 0 && i < m_width * m_height)
		return m_pixels[i];
	else
	{
		std::cout << "Out of bounds error in " << m_fileName << ": " << i << "\n";
		return 0xFFFF00FF;
	}
}

uint32_t Texture::getTexel(int x, int y)
{
	if (x >= 0 && x < m_width && y >= 0 && y < m_height)
		return m_pixels[y * m_width + x];
	else
	{
		// std::cout << "Out of bounds error in " << m_fileName << " at (" << x << ", " << y << ")\n";
		return 0xFFFF00FF;
	}
}

uint32_t Texture::getTexel(float normX, float normY)
{
	int x{ static_cast<int>(normX * m_width) };
	int y{ static_cast<int>(normY * m_height) };
	return getTexel(x, y);
}

Texture::~Texture()
{
	delete[] m_pixels;
}

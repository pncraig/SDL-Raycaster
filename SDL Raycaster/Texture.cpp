#include "Texture.h"
#include "SDL.h"
#include "SDL_image.h"
#include <iostream>

Texture::Texture(const std::string& fileName, int pixelFormat)
{
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

Texture::~Texture()
{
	delete[] m_pixels;
}

uint32_t Texture::operator[](int i)
{
	if (i >= 0 && i < m_width * m_height)
		return m_pixels[i];
	else
	{
		// std::cout << "Out of bounds error: " << i << "\n";
		return 0xFFFF00FF;
	}
}
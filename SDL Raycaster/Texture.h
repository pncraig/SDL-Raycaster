#ifndef TEXTURE_H
#define TEXTURE_H

#include "SDL.h"
#include <iostream>
#include "SDL_image.h"


class Texture
{
private:
	uint32_t* m_pixels{};
	int m_width{};
	int m_height{};
	std::string m_fileName;
	static bool bilinearFiltering;
public:
	Texture(const std::string& fileName, int pixelFormat);
	Texture(const std::string& fileName, const SDL_Rect& section, int pixelFormat);
	Texture() {}
	Texture(const Texture& t);

	Texture& operator=(const Texture& t);

	int width() { return m_width; }
	int height() { return m_height; }
	std::string fileName() { return m_fileName; }

	uint32_t operator[](int i);
	uint32_t getTexel(int x, int y);				// Access a texel using coordinates in a plane with a width of m_width and height of m_height
	uint32_t getTexel(double normX, double normY);	// Access a texel using normalized coordinates

	static void turnBilinearFilteringOn() { bilinearFiltering = true; }

	~Texture();
};

#endif


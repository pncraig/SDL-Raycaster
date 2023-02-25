#include "Screen.h"

Screen::Screen(SDL_Renderer* renderTarget, int width, int height)
	: m_width{ width }, m_height{ height }
{
	m_pixels = new uint32_t[m_width * m_height];
	m_renderTarget = renderTarget;
	m_frameBuffer = SDL_CreateTexture(renderTarget, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, m_width, m_height);
}

uint32_t Screen::getPixel(int x, int y)
{
	if (x >= 0 && x < m_width && y >= 0 && y < m_height)
		return m_pixels[y * m_width + x];
	else
	{
		std::cout << "Out of bounds error in screen at (" << x << ", " << y << ")\n";
		return 0xFFFF00FF;
	}
}

uint32_t Screen::setPixel(uint32_t color, int x, int y)
{
	if (x >= 0 && x < m_width && y >= 0 && y < m_height)
	{
		uint32_t prevPixel{ getPixel(x, y) };
		m_pixels[y * m_width + x] = color;
		return prevPixel;
	}
	else
	{
		std::cout << "Out of bounds error in screen at (" << x << ", " << y << ")\n";
		return 0xFFFF00FF;
	}
}

void Screen::paintScreen(uint32_t color)
{
	for (int i{ 0 }; i < m_width * m_height; i++)
		m_pixels[i] = color;
}

void Screen::updateScreenTexture()
{
	SDL_UpdateTexture(m_frameBuffer, NULL, m_pixels, m_width * sizeof(uint32_t));
}

void Screen::drawToWindow(SDL_Rect destRect)
{
	SDL_RenderCopy(m_renderTarget, m_frameBuffer, NULL, &destRect);
}

Screen::~Screen()
{
	delete[] m_pixels;
	SDL_DestroyTexture(m_frameBuffer);
}
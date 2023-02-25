#ifndef SCREEN_H
#define SCREEN_H

#include "SDL.h"
#include <iostream>

class Screen
{
private:
	uint32_t* m_pixels;
	SDL_Texture* m_frameBuffer;
	SDL_Renderer* m_renderTarget;
	const int m_width{};
	const int m_height{};

public:
	Screen(SDL_Renderer* renderTarget, int width, int height);

	int width() { return m_width; }
	int height() { return m_height; }

	uint32_t getPixel(int x, int y);
	uint32_t setPixel(uint32_t color, int x, int y);

	void paintScreen(uint32_t color);

	void updateScreenTexture();
	void drawToWindow(SDL_Rect destRect);

	~Screen();
};

#endif
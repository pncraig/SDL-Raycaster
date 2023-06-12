#ifndef SCREENHANDLER_H
#define SCREENHANDLER_H

#include <iostream>

class ScreenHandler
{
private:
	uint32_t* m_ScreenPixels;
	const int m_ScreenWidth{};
	const int m_ScreenHeight{};

public:
	ScreenHandler(int width, int height);

	bool init();

	int width() { return m_ScreenWidth; }
	int height() { return m_ScreenHeight; }

	uint32_t getPixel(int x, int y);
	uint32_t setPixel(int x, int y, uint32_t color);

	void paintScreen(uint32_t color);

	uint32_t* getPointerToScreen();

	~ScreenHandler();
};

#endif
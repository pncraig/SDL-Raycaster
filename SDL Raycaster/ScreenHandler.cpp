#include "ScreenHandler.h"

ScreenHandler::ScreenHandler(int width, int height)
	: m_ScreenWidth{ width }, m_ScreenHeight{ height }
{
}

bool ScreenHandler::init()
{
	m_ScreenPixels = new uint32_t[m_ScreenWidth * m_ScreenHeight];
	if (m_ScreenPixels == nullptr)
	{
		std::cout << "Unable to allocate memory for the screen.\n";
		return false;
	}

	return true;
}

uint32_t ScreenHandler::getPixel(int x, int y)
{
	if (x >= 0 && x < m_ScreenWidth && y >= 0 && y < m_ScreenHeight)
		return m_ScreenPixels[y * m_ScreenWidth + x];
	else
	{
		std::cout << "Out of bounds error in screen at (" << x << ", " << y << ")\n";
		return 0xFFFF00FF;
	}
}

uint32_t ScreenHandler::setPixel(int x, int y, uint32_t color)
{
	if (x >= 0 && x < m_ScreenWidth && y >= 0 && y < m_ScreenHeight)
	{
		uint32_t prevPixel{ getPixel(x, y) };
		m_ScreenPixels[y * m_ScreenWidth + x] = color;
		return prevPixel;
	}
	else
	{
		std::cout << "Out of bounds error in screen at (" << x << ", " << y << ")\n";
		return 0xFFFF00FF;
	}
}

void ScreenHandler::paintScreen(uint32_t color)
{
	for (int i{ 0 }; i < m_ScreenWidth * m_ScreenHeight; i++)
		m_ScreenPixels[i] = color;
}

uint32_t* ScreenHandler::getPointerToScreen()
{
	return m_ScreenPixels;
}

ScreenHandler::~ScreenHandler()
{
	delete[] m_ScreenPixels;
}
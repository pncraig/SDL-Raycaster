#include "Player.h"

Player::Player()
{

}

Player::Player(double x, double y, double z, double a, int h = 0)
	:m_PlayerX{ x }, m_PlayerY{ y }, m_PlayerZ{ z }, m_PlayerA{ a }, m_PlayerHeight{ h }, m_PlayerHorizonOffset{ 0.0 }
{
	m_PlayerSpeed = 100.0;
}

double Player::getX() { return m_PlayerX; }
double Player::getY() { return m_PlayerY; }
double Player::getZ() { return m_PlayerZ; }
double Player::getA() { return m_PlayerA; }

int Player::getHeight() { return m_PlayerHeight; }
double Player::getHorizonOffset() { return m_PlayerHorizonOffset; }

void Player::setX(double x) { m_PlayerX = x; }
void Player::setY(double y) { m_PlayerY = y; }
void Player::setZ(double z) { m_PlayerZ = z; }
void Player::setA(double a) { m_PlayerA = a; }

void Player::addX(double dx) { m_PlayerX += dx; }
void Player::addY(double dy) { m_PlayerY += dy; }
void Player::addZ(double dz) { m_PlayerZ += dz; }
void Player::addA(double da) { m_PlayerA += da; }
void Player::addVec(double dx, double dy, double dz)
{
	m_PlayerX += dx;
	m_PlayerY += dy;
	m_PlayerZ += dz;
}

void Player::moveForward(double deltaTime)
{
	// ySpeed is negative because we're using the typical degree layout (unit circle), but y is increasing downward in out coordinate grid
	//	     90				 ----------> x
	//       |				|
	//180 ---+--- 0			|
	//		 |				v
	//		270				y
	// Because sine is positive in the first two quadrants on the unit circle, but that direction is down in our coordinate grid,
	// we need to make it negative to flip the sign

	double xSpeed{ m_PlayerSpeed * cos(util::radians(m_PlayerA)) };
	double ySpeed{ -m_PlayerSpeed * sin(util::radians(m_PlayerA)) };

	m_PlayerX += xSpeed * deltaTime;
	m_PlayerY += ySpeed * deltaTime;
}

void Player::moveBackward(double deltaTime)
{
	double xSpeed{ -m_PlayerSpeed * cos(util::radians(m_PlayerA)) };
	double ySpeed{ m_PlayerSpeed * sin(util::radians(m_PlayerA)) };

	m_PlayerX += xSpeed * deltaTime;
	m_PlayerY += ySpeed * deltaTime;
}

void Player::rotateLeft(double deltaTime)
{
	m_PlayerA += m_PlayerSpeed * deltaTime;
}

void Player::rotateRight(double deltaTime)
{
	m_PlayerA -= m_PlayerSpeed * deltaTime;
}

void Player::setHeight(int h) { m_PlayerHeight = h; }
void Player::setHorizonOffset(double o) { m_PlayerHorizonOffset = o; }
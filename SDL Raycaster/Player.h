#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>
#include "Utility.h"

class Player
{
private:
	double m_PlayerX{};
	double m_PlayerY{};
	double m_PlayerZ{};
	double m_PlayerA{};

	int m_PlayerHeight{};
	double m_PlayerHorizonOffset{};
	double m_PlayerSpeed{};

public:
	Player();
	Player(double x, double y, double z, double a, int h);

	double getX();
	double getY();
	double getZ();
	double getA();

	int getHeight();
	double getHorizonOffset();

	void setX(double x);
	void setY(double y);
	void setZ(double z);
	void setA(double a);

	void addX(double dx);
	void addY(double dy);
	void addZ(double dz);
	void addA(double da);
	void addVec(double dx, double dy, double dz);

	void moveForward(double deltaTime);
	void moveBackward(double deltaTime);
	void rotateLeft(double deltaTime);
	void rotateRight(double deltaTime);

	void setHeight(int h);
	void setHorizonOffset(double o);
};

#endif
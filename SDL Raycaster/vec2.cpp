#include "vec2.h"
#include <iostream>

float vec2::mag()
{
	return sqrtf(x * x + y * y);
}

float vec2::dir()
{
	return atan2f(y, x) * (180.0f / 3.141592653f);
}

void vec2::rotate(float angle, const vec2& point)
{
	angle *= 3.141592653f / 180.0f;

	x -= point.x;
	y -= point.y;

	float cos{ cosf(angle) };
	float sin{ sinf(angle) };

	float xPrime = x * cos - y * sin;
	float yPrime = x * sin + y * cos;

	x = point.x + xPrime;
	y = point.y + yPrime;
}

void vec2::norm()
{
	if (mag() != 0.0f)
	{
		float invMag{ 1 / mag() };
		x *= invMag;
		y *= invMag;
	}
}

// Class operator overloading
void vec2::operator+=(const vec2& a)
{
	x += a.x;
	y += a.y;
}

void vec2::operator-=(const vec2& a)
{
	x -= a.x;
	y -= a.y;
}

void vec2::operator*=(float s)
{
	x *= s;
	y *= s;
}

void vec2::operator/=(float s)
{
	x /= s;
	y /= s;
}

// Friend functions
std::ostream& operator<<(std::ostream& out, const vec2& vec)
{
	out << "<" << vec.x << ", " << vec.y << ">";
	return out;
}

vec2 operator+(const vec2& a, const vec2& b)
{
	return vec2{ a.x + b.x, a.y + b.y };
}

vec2 operator-(const vec2& a, const vec2& b)
{
	return vec2{ a.x - b.x, a.y - b.y };
}

vec2 operator*(const vec2& a, float s)
{
	return vec2{ a.x * s, a.y * s };
}

vec2 operator/(const vec2& a, float s)
{
	return vec2{ a.x / s, a.y / s };
}
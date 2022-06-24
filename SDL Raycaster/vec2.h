#ifndef VEC2_H
#define VEC2_H

#include<iostream>

class vec2
{
public:
	float x{};
	float y{};

	vec2(float i_x = 0.0f, float i_y = 0.0f)
		: x{ i_x }, y{ i_y }
	{

	}

	float mag();
	float dir();
	void rotate(float angle, const vec2& point = { 0.0f, 0.0f });
	void norm();

	void operator+=(const vec2& a);
	void operator-=(const vec2& a);
	void operator*=(float s);
	void operator/=(float s);

	friend std::ostream& operator<<(std::ostream& out, const vec2& vec);
	friend vec2 operator+(const vec2& a, const vec2& b);
	friend vec2 operator-(const vec2& a, const vec2& b);
	friend vec2 operator*(const vec2& a, float s);
	friend vec2 operator/(const vec2& a, float s);
};

#endif

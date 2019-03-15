#include "hzpch.h"
#include "Color.h"

namespace Hazel {

	const Color Color::Black(0, 0, 0);
	const Color Color::White(255, 255, 255);
	const Color Color::Red(255, 0, 0);
	const Color Color::Green(0, 255, 0);
	const Color Color::Blue(0, 0, 255);
	const Color Color::Yellow(255, 255, 0);
	const Color Color::Magenta(255, 0, 255);
	const Color Color::Cyan(0, 255, 255);
	const Color Color::Transparent(0, 0, 0, 0);

	Color Color::operator+(const Color& other)
	{
		unsigned char r = std::min(int(this->R) + other.R, 255);
		unsigned char g = std::min(int(this->G) + other.G, 255);
		unsigned char b = std::min(int(this->B) + other.B, 255);
		unsigned char a = std::min(int(this->A) + other.A, 255);
		return Color(r, g, b, a);
	}

	Color Color::operator-(const Color& other)
	{
		unsigned char r = std::max(int(this->R) - other.R, 0);
		unsigned char g = std::max(int(this->G) - other.G, 0);
		unsigned char b = std::max(int(this->B) - other.B, 0);
		unsigned char a = std::max(int(this->A) - other.A, 0);
		return Color(r, g, b, a);
	}

	Color Color::operator*(const Color& other)
	{
		unsigned char r = int(this->R) * other.R / 255;
		unsigned char g = int(this->G) * other.G / 255;
		unsigned char b = int(this->B) * other.B / 255;
		unsigned char a = int(this->A) * other.A / 255;
		return Color(r, g, b, a);
	}

}
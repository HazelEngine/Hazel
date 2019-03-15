#pragma once

#include "Hazel/Core.h"

namespace Hazel {

	class HAZEL_API Color
	{
	public:
		//! CTOR/DTOR:
		Color(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0, unsigned char a = 255);
		virtual ~Color();

		//! SERVICES:
		unsigned int ToInteger() const;

		//! OPERATORS:
		// Colors can be added and modulated (multiplied)
		// using these overloaded operators.
		Color operator+(const Color& other);
		Color operator-(const Color& other);
		Color operator*(const Color& other);
		bool operator==(const Color& other);
		bool operator!=(const Color& other);

		//! PREDEFINED COLORS:
		// Common colors are defined as static variables.
		static const Color Black;
		static const Color White;
		static const Color Red;
		static const Color Green;
		static const Color Blue;
		static const Color Yellow;
		static const Color Magenta;
		static const Color Cyan;
		static const Color Transparent;

		//! MEMBERS:
		// Components are stored in the range [0 -> 255].
		unsigned char R; // r component
		unsigned char G; // g component
		unsigned char B; // b component
		unsigned char A; // a (opacity) component
	};

	////////////////////////////////////////////////////////////////////////////////
	// Color inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline Color::Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) :
		R(r), G(g), B(b), A(a)
	{
	}

	inline Color::~Color()
	{
	}

	inline unsigned int Color::ToInteger() const
	{
		return (R) | (G << 8) | (B << 16) | (A << 24);
	}

	inline bool Color::operator==(const Color& other)
	{
		return (R == other.R && G == other.G && B == other.B && A == other.A);
	}

	inline bool Color::operator!=(const Color& other)
	{
		return (R != other.R || G != other.G || B != other.B || A != other.A);
	}

}
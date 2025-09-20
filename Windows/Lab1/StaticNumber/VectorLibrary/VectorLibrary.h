#pragma once

#ifdef VECTORLIBRARY_EXPORTS
	#define VECTORLIBRARY_API __declspec(dllexport)
#else
	#define VECTORLIBRARY_API __declspec(dllimport)
#endif

#include "NumberLibrary.h"

using namespace NumberLibrary;

class VECTORLIBRARY_API Vector {
private: 
	Number x;
	Number y;
public:
	Vector();
	Vector(const Number& x, const Number& y);

	Number getX() const;
	Number getY() const;

	Number getRadius() const;
	Number getAngle() const;

	Vector operator+(const Vector& other) const;
	Vector operator-(const Vector& other) const;

	static const Vector zero;
	static const Vector one;
};
#include "pch.h"
#include <utility>
#include <limits.h>
#include "VectorLibrary.h"


Vector::Vector() : x(createNumber(0.0)), y(createNumber(0.0)) {}
Vector::Vector(const Number& x, const Number& y) : x(x), y(y) {}

Number Vector::getX() const {
	return x;
}

Number Vector::getY() const {
	return y;
}

Number Vector::getRadius() const {
	return sqrt(x * x + y * y);
}

Number Vector::getAngle() const
{
	if (x.get() == 0.0 && y.get() == 0.0) {
		return Number(0.0);
	}
	return arctan(y / x);
}

Vector Vector::operator+(const Vector& other) const
{
	return Vector(x + other.x, y + other.y);
}

Vector Vector::operator-(const Vector& other) const
{
	return Vector(x - other.x, y - other.y);
}

const Vector Vector::zero(Number(0.0), Number(0.0));
const Vector Vector::one(Number(1.0), Number(1.0));
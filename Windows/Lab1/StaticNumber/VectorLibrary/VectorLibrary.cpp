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
    double dx = x.get();
    double dy = y.get();

    if (dx == 0.0 && dy == 0.0) {
        return Number::zero;
    }

    Number rad;
    Number ratio = dy / dx;
    Number half_pi = arctan(createNumber(1.0));
    Number pi = half_pi * createNumber(4.0);

    if (dx > 0.0) {
        rad = arctan(ratio);
    }
    else if (dx < 0.0 && dy >= 0.0) {
        rad = arctan(ratio) + pi;
    }
    else if (dx < 0.0 && dy < 0.0) {
        rad = arctan(ratio) - pi;
    }
    else if (dx == 0.0 && dy > 0.0) {
        rad = half_pi * createNumber(2.0);
    }
    else {
        rad = half_pi * createNumber(-2.0);
    }

    Number deg = rad * createNumber(180.0) / pi;

    if (deg.get() < 0.0) {
        deg = deg + createNumber(360.0);
    }

    return deg;
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
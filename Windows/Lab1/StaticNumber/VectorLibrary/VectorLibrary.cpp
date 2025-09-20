#include "pch.h"
#include <utility>
#include <limits.h>
#include "VectorLibrary.h"
#include <cmath>


Vector::Vector() : x(createNumber(0.0)), y(createNumber(0.0)) {}
Vector::Vector(const Number& x, const Number& y) : x(x), y(y) {}

Number Vector::getX() const {
	return x;
}

Number Vector::getY() const {
	return y;
}

Number Vector::getRadius() const {
	return createNumber(sqrt(x.get() * x.get() + y.get() * y.get()));
}

Number Vector::getAngle() const
{
	if (x.get() == 0.0 && y.get() == 0.0) {
		return Number(0.0);
	}
	return createNumber(atan2(y.get(), x.get()));
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

//Vector* createVector(const Number& x, const Number& y) {
//	return new Vector(x, y);
//}
//
//void deleteVector(Vector* vec) {
//	delete vec;
//}
//
//Number getVectorX(const Vector* vec) {
//	return vec->getX();
//}
//
//Number getVectorY(const Vector* vec) {
//	return vec->getY();
//}
//
//Number getVectorRadius(const Vector* vec) {
//	return vec->getRadius();
//}
//
//Number getVectorAngle(const Vector* vec) {
//	return vec->getAngle();
//}
//
//Vector* addVectors(const Vector* vec1, const Vector* vec2) {
//	Vector* result = new Vector(*vec1 + *vec2);
//	return result;
//}
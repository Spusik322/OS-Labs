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

//extern "C" VECTORLIBRARY_API Vector* createVector(const Number& x, const Number& y);
//extern "C" VECTORLIBRARY_API void deleteVector(Vector* vec);
//extern "C" VECTORLIBRARY_API Number getVectorX(const Vector* vec);
//extern "C" VECTORLIBRARY_API Number getVectorY(const Vector* vec);
//extern "C" VECTORLIBRARY_API Number getVectorRadius(const Vector* vec);
//extern "C" VECTORLIBRARY_API Number getVectorAngle(const Vector* vec);
//extern "C" VECTORLIBRARY_API Vector* addVectors(const Vector* vec1, const Vector* vec2);
#pragma once
#include <iostream>
#include <cmath>

namespace NumberLibrary
{
    class Number {
    private:
        double value;

    public:
        Number(double val = 0.0);
        double get() const;
        
        Number operator+(const Number& other) const;
        Number operator-(const Number& other) const;
        Number operator/(const Number& other) const;
        Number operator*(const Number& other) const;

        static const Number zero;
        static const Number one;

        friend std::ostream& operator<<(std::ostream& os, const Number& num);
        friend std::istream& operator>>(std::istream& is, Number& num);
    };

    Number sqrt(const Number& num);
    Number arctan(const Number& num);
    Number createNumber(double value);
}
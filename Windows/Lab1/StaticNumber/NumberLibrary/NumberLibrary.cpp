#include "NumberLibrary.h"
#include <stdexcept>

namespace NumberLibrary
{
	Number::Number(double val) : value(val) {}

	double Number::get() const {
		return value;
	}

	Number Number::operator+(const Number& other) const {
		return Number(value + other.value);
	}

	Number Number::operator-(const Number& other) const {
		return Number(value - other.value);
	}

	Number Number::operator/(const Number& other) const {
		if (other.value == 0.0) {
			throw std::runtime_error("Division by zero");
		}
		return Number(value / other.value);
	}

	Number Number::operator*(const Number& other) const {
		return Number(value * other.value);
	}

	const Number Number::zero(0.0);
	const Number Number::one(1.0);

	std::ostream& operator<<(std::ostream& os, const Number& num) {
		os << num.value;
		return os;
	}

	std::istream& operator>>(std::istream& is, Number& num) {
		is >> num.value;
		return is;
	}

	Number sqrt(const Number& num)
	{
		return Number(std::sqrt(num.get()));
	}

	Number arctan(const Number& num)
	{
		return Number(std::atan(num.get()));
	}

	Number createNumber(double value) {
		return Number(value);
	}
}
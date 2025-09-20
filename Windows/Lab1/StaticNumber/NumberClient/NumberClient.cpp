#include <iostream>
#include "NumberLibrary.h"

using namespace NumberLibrary;

int main()
{
    try {
        std::cout << Number::zero << std::endl;
        std::cout << Number::one << std::endl;

        Number a = createNumber(10.5);
        Number b = createNumber(2.5);

        std::cout << "a + b = " << (a + b) << std::endl;
        std::cout << "a - b = " << (a - b) << std::endl;
        std::cout << "a * b = " << (a * b) << std::endl;
        std::cout << "a / b = " << (a / b) << std::endl;

        Number c;
        std::cout << "Enter a number: ";
        std::cin >> c;
        std::cout << "You entered: " << c << std::endl;

        Number d = createNumber(5.0);
        Number e = Number::zero;
        std::cout << "d / e = " << (d / e) << std::endl;

    }
    catch (const std::runtime_error& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
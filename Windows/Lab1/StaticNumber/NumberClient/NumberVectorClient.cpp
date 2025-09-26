#include <iostream>
#include "NumberLibrary.h"
#include "VectorLibrary.h"

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

        Number x = createNumber(9.0);
        Number y = sqrt(x);              
        Number z = arctan(createNumber(1)); 

        std::cout << "sqrt(9)   = " << y << std::endl;
        std::cout << "arctan(1) = " << z << std::endl;

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
    Number x1 = createNumber(3.0);
    Number y1 = createNumber(4.0);
    Number x2 = createNumber(1.0);
    Number y2 = createNumber(2.0);
        
    Vector v1 = Vector(x1, y1);
    Vector v2 = Vector(x2, y2);
        
    Number v1x = v1.getX();
    Number v1y = v1.getY();
    std::cout << "Vector 1: (" << v1x << ", " << v1y << ")" << std::endl;
        
    Number v2x = v2.getX();
    Number v2y = v2.getY();
    std::cout << "Vector 2: (" << v2x << ", " << v2y << ")" << std::endl;

    Number radius = v1.getRadius();
    Number angle = v1.getAngle();
 
    std::cout << "Vector 1 radius: " << radius << std::endl;
    std::cout << "Vector 1 angle: " << angle << " in degrees" << std::endl;
        
    Vector sum = v1 + v2;
    Vector div = v1 - v2;
    Number sumX = sum.getX();
    Number sumY = sum.getY();
    std::cout << "Sum of vectors: (" << sumX << ", " << sumY << ")" << std::endl;
    Number divX = div.getX();
    Number divY = div.getY();
    std::cout << "Div of vectors: (" << divX << ", " << divY << ")" << std::endl;
        
    std::cout << "Zero vector: (" << Vector::zero.getX() << ", " << Vector::zero.getY() << ")" << std::endl;
    std::cout << "One vector: (" << Vector::one.getX() << ", " << Vector::one.getY() << ")" << std::endl;
    return 0;
}
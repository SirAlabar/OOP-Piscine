#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include "shape.hpp"
#include <cmath>

class Triangle : public Shape
{
private:
    double side_a;
    double side_b;
    double side_c;

public:
    Triangle(double a, double b, double c)
        : side_a(a), side_b(b), side_c(c)
    {
    }

    double area() const
    {
        double s = (side_a + side_b + side_c) / 2.0;
        return std::sqrt(s * (s - side_a) * (s - side_b) * (s - side_c));
    }

    double perimeter() const
    {
        return side_a + side_b + side_c;
    }
};

#endif
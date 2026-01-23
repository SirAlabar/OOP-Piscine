#ifndef SHAPE_HPP
#define SHAPE_HPP

class Shape
{
public:
    virtual double area() const = 0;
    virtual double perimeter() const = 0;
    virtual ~Shape() {}
};

#endif
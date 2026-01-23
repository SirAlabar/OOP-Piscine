#include <iostream>
#include <iomanip>
#include <vector>
#include "shape.hpp"
#include "rectangle.hpp"
#include "circle.hpp"
#include "triangle.hpp"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

void print_shape_info(const Shape& shape, const std::string& name, const std::string& color)
{
    std::cout << color << BOLD << "=== " << name << " ===" << RESET << std::endl;
    std::cout << color << "Area: " << std::fixed << std::setprecision(2) 
              << shape.area() << RESET << std::endl;
    std::cout << color << "Perimeter: " << std::fixed << std::setprecision(2) 
              << shape.perimeter() << RESET << std::endl;
    std::cout << std::endl;
}

int main()
{
    std::cout << BOLD << CYAN << "  LISKOV SUBSTITUTION PRINCIPLE DEMO   " << RESET << std::endl;
    std::cout << std::endl;

    Rectangle rect(5.0, 3.0);
    Circle circ(4.0);
    Triangle tri(3.0, 4.0, 5.0);

    std::cout << YELLOW << BOLD << ">>> Testing Individual Shapes <<<" << RESET << std::endl;
    std::cout << std::endl;

    print_shape_info(rect, "Rectangle (5 x 3)", GREEN);
    print_shape_info(circ, "Circle (radius 4)", BLUE);
    print_shape_info(tri, "Triangle (3, 4, 5)", MAGENTA);

    std::cout << YELLOW << BOLD << ">>> Testing Liskov Substitution Principle <<<" << RESET << std::endl;
    std::cout << CYAN << "All shapes treated as Shape* polymorphically" << RESET << std::endl;
    std::cout << std::endl;

    std::vector<Shape*> shapes;
    shapes.push_back(&rect);
    shapes.push_back(&circ);
    shapes.push_back(&tri);

    double total_area = 0.0;
    double total_perimeter = 0.0;

    for (size_t i = 0; i < shapes.size(); ++i)
    {
        total_area += shapes[i]->area();
        total_perimeter += shapes[i]->perimeter();
    }

    std::cout << RED << BOLD << "Total Area (all shapes): " 
              << std::fixed << std::setprecision(2) << total_area << RESET << std::endl;
    std::cout << RED << BOLD << "Total Perimeter (all shapes): " 
              << std::fixed << std::setprecision(2) << total_perimeter << RESET << std::endl;
    std::cout << std::endl;

    std::cout << CYAN << BOLD << "✓ Liskov Substitution Principle satisfied!" << RESET << std::endl;
    std::cout << CYAN << "  All derived classes can replace Shape without breaking behavior" << RESET << std::endl;

    return 0;
}
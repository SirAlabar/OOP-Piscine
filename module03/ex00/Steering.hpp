#ifndef STEERING_HPP
#define STEERING_HPP

#include <iostream>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

class Steering
{
private:
    int wheelAngle;

public:
    Steering() : wheelAngle(0) {}

    void turnWheel(int angle)
    {
        wheelAngle = angle;
        if (angle > 0)
            std::cout << CYAN << "🏎️ Drifting RIGHT at " << wheelAngle << " degrees! Smooth!" << RESET << std::endl;
        else if (angle < 0)
            std::cout << CYAN << "🏎️ Drifting LEFT at " << wheelAngle << " degrees! Smooth!" << RESET << std::endl;
        else
            std::cout << GREEN << "Going straight like a champion!" << RESET << std::endl;
    }

    void straighten()
    {
        wheelAngle = 0;
        std::cout << GREEN << BOLD << "✨ Wheels straightened! Back on track!" << RESET << std::endl;
    }
};

#endif
#ifndef BRAKES_HPP
#define BRAKES_HPP

#include <iostream>
#include "Engine.hpp"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

class Brakes
{
private:
    int currentForce;

public:
    Brakes() : currentForce(0) {}

    void applyForce(int force, Engine& engine)
    {
        currentForce = force;
        std::cout << YELLOW << "🛑 *SCREEEECH* Brakes engaged with force " << force << "!" << RESET << std::endl;
        engine.reduceSpeed(force / 10);
        std::cout << BLUE << "⬇️ Slowing down to " << engine.getSpeed() << " mph!" << RESET << std::endl;
    }

    void applyEmergencyBrakes(Engine& engine)
    {
        currentForce = 100;
        std::cout << RED << BOLD << "🚨 EMERGENCY STOP! *SCREEEEEEEECH* 🚨" << RESET << std::endl;
        engine.stopSpeed();
        std::cout << RED << BOLD << "💥 Full stop! Speed: 0 mph!" << RESET << std::endl;
    }
};

#endif
#ifndef TRANSMISSION_HPP
#define TRANSMISSION_HPP

#include <iostream>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

class Transmission
{
private:
    int currentGear;
    bool inReverse;

public:
    Transmission() : currentGear(0), inReverse(false) {}

    void shiftUp()
    {
        if (inReverse)
        {
            std::cout << RED << "Can't shift up in reverse! What are you thinking?!" << RESET << std::endl;
            return;
        }
        if (currentGear < 6)
        {
            currentGear++;
            std::cout << GREEN << "🔧 *CLICK* Shifted to gear " << currentGear << "! Let's go!" << RESET << std::endl;
        }
        else
        {
            std::cout << YELLOW << "Already maxed out, buddy! Can't go higher!" << RESET << std::endl;
        }
    }

    void shiftDown()
    {
        if (inReverse)
        {
            std::cout << RED << "Can't downshift in reverse! C'mon!" << RESET << std::endl;
            return;
        }
        if (currentGear > 0)
        {
            currentGear--;
            std::cout << BLUE << "⬇️ *clunk* Dropped to gear " << currentGear << RESET << std::endl;
        }
        else
        {
            std::cout << YELLOW << "Already in neutral, can't go lower!" << RESET << std::endl;
        }
    }

    void engageReverse()
    {
        if (currentGear == 0)
        {
            inReverse = true;
            std::cout << MAGENTA << "🔄 Switching to reverse! Backing up like a champ!" << RESET << std::endl;
        }
        else
        {
            std::cout << RED << "Gotta be in neutral first! Don't break the gearbox!" << RESET << std::endl;
        }
    }
};

#endif
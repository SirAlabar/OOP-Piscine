#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <iostream>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

class Engine
{
private:
    bool running;
    int speed;

public:
    Engine() : running(false), speed(0) {}

    void start()
    {
        if (!running)
        {
            running = true;
            std::cout << GREEN << BOLD << "🏁 Ka-chow! Lightning's engine roaring!" << RESET << std::endl;
        }
        else
        {
            std::cout << YELLOW << "Engine's already revving, buddy!" << RESET << std::endl;
        }
    }

    void stop()
    {
        if (running)
        {
            running = false;
            speed = 0;
            std::cout << RED << "Engine cooling down... *vroooom fades*" << RESET << std::endl;
        }
        else
        {
            std::cout << YELLOW << "Engine's already off, pal!" << RESET << std::endl;
        }
    }

    void accelerate(int speedIncrease)
    {
        if (running)
        {
            speed += speedIncrease;
            std::cout << CYAN << BOLD << "⚡ SPEED! I AM SPEED! Current speed: " << speed << " mph!" << RESET << std::endl;
        }
        else
        {
            std::cout << RED << "Can't go fast without starting the engine first, rookie!" << RESET << std::endl;
        }
    }

    bool isRunning() const
    {
        return running;
    }

    int getSpeed() const
    {
        return speed;
    }

    void reduceSpeed(int amount)
    {
        speed -= amount;
        if (speed < 0)
        {
            speed = 0;
        }
    }

    void stopSpeed()
    {
        speed = 0;
    }
};

#endif
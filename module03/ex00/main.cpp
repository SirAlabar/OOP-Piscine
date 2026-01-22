#include "Car.hpp"
#include <iostream>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

int main()
{
    Car lightning;

    std::cout << MAGENTA << BOLD << "\n🏁 ========== LIGHTNING MCQUEEN TEST DRIVE! ========== 🏁\n" << RESET << std::endl;

    std::cout << CYAN << BOLD << "=== Starting the Race Machine ===" << RESET << std::endl;
    lightning.start();
    lightning.start();
    
    std::cout << CYAN << BOLD << "\n=== Time to GO FAST! ===" << RESET << std::endl;
    lightning.accelerate(20);
    lightning.accelerate(30);
    lightning.accelerate(25);
    
    std::cout << CYAN << BOLD << "\n=== Shifting Through the Gears! ===" << RESET << std::endl;
    lightning.shift_gears_up();
    lightning.shift_gears_up();
    lightning.shift_gears_up();
    
    std::cout << CYAN << BOLD << "\n=== Taking the Turns! ===" << RESET << std::endl;
    lightning.turn_wheel(45);
    lightning.turn_wheel(-30);
    
    std::cout << CYAN << BOLD << "\n=== Back on the Straight ===" << RESET << std::endl;
    lightning.straighten_wheels();
    
    std::cout << CYAN << BOLD << "\n=== Slowing Down for the Pit ===" << RESET << std::endl;
    lightning.apply_force_on_brakes(30);
    lightning.apply_force_on_brakes(20);
    
    std::cout << CYAN << BOLD << "\n=== Downshifting ===" << RESET << std::endl;
    lightning.shift_gears_down();
    lightning.shift_gears_down();
    lightning.shift_gears_down();
    lightning.shift_gears_down();
    
    std::cout << CYAN << BOLD << "\n=== Backing Up in the Pit ===" << RESET << std::endl;
    lightning.reverse();
    
    std::cout << CYAN << BOLD << "\n=== CRASH INCOMING! ===" << RESET << std::endl;
    lightning.accelerate(50);
    lightning.apply_emergency_brakes();
    
    std::cout << CYAN << BOLD << "\n=== Race Over, Shutting Down ===" << RESET << std::endl;
    lightning.stop();
    
    std::cout << CYAN << BOLD << "\n=== Trying to Race with No Engine ===" << RESET << std::endl;
    lightning.accelerate(20);

    std::cout << MAGENTA << BOLD << "\n🏁 ========== KA-CHOW! RACE COMPLETE! ========== 🏁\n" << RESET << std::endl;

    return 0;
}
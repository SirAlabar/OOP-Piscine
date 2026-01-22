#include "Shovel.hpp"
#include <iostream>

#define GREEN "\033[32m"
#define RESET "\033[0m"

Shovel::Shovel() : ATool()
{
	std::cout << GREEN << "Shovel created" << RESET << std::endl;
}

Shovel::Shovel(const Shovel& other) : ATool()
{
	numberOfUses = other.numberOfUses;
	std::cout << GREEN << "Shovel copied" << RESET << std::endl;
}

Shovel& Shovel::operator=(const Shovel& other)
{
	if (this != &other)
	{
		numberOfUses = other.numberOfUses;
	}
	return *this;
}

Shovel::~Shovel()
{
	std::cout << GREEN << "Shovel destroyed" << RESET << std::endl;
}

void Shovel::use()
{
	std::cout << GREEN << "Shovel used (uses: " << ++numberOfUses << ")" << RESET << std::endl;
}
#include "Hammer.hpp"
#include <iostream>

#define GREEN "\033[32m"
#define RESET "\033[0m"

Hammer::Hammer() : ATool()
{
	std::cout << GREEN << "Hammer created" << RESET << std::endl;
}

Hammer::Hammer(const Hammer& other) : ATool()
{
	numberOfUses = other.numberOfUses;
	std::cout << GREEN << "Hammer copied" << RESET << std::endl;
}

Hammer& Hammer::operator=(const Hammer& other)
{
	if (this != &other)
	{
		numberOfUses = other.numberOfUses;
	}
	return *this;
}

Hammer::~Hammer()
{
	std::cout << GREEN << "Hammer destroyed" << RESET << std::endl;
}

void Hammer::use()
{
	std::cout << GREEN << "Hammer used (uses: " << ++numberOfUses << ")" << RESET << std::endl;
}
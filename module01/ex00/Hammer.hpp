#ifndef HAMMER_HPP
#define HAMMER_HPP

#include "ATool.hpp"

class Hammer : public ATool
{
public:
	Hammer();
	Hammer(const Hammer& other);
	Hammer& operator=(const Hammer& other);
	~Hammer();

	void use();
};

#endif
#ifndef SHOVEL_HPP
#define SHOVEL_HPP

#include "ATool.hpp"

class Shovel : public ATool
{
public:
	Shovel();
	Shovel(const Shovel& other);
	Shovel& operator=(const Shovel& other);
	~Shovel();

	void use();
};

#endif
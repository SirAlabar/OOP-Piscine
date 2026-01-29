#include "people/Staff.hpp"
#include "forms/Form.hpp"
#include <iostream>

#define YELLOW "\033[33m"
#define RESET "\033[0m"

Staff::Staff(std::string p_name) : Person(p_name)
{
}

Staff::~Staff()
{
}

void Staff::sign(Form* p_form)
{
	if (p_form)
	{
		p_form->sign();
		std::cout << YELLOW << getName() << " signed the form" << RESET << std::endl;
	}
}
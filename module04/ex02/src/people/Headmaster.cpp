#include "people/Headmaster.hpp"
#include "forms/Form.hpp"
#include <iostream>

#define MAGENTA "\033[35m"
#define RESET "\033[0m"

Headmaster::Headmaster(std::string p_name) : Staff(p_name)
{
}

Headmaster::~Headmaster()
{
	for (std::vector<Form*>::iterator it = _formToValidate.begin(); it != _formToValidate.end(); ++it)
	{
		delete *it;
	}
	_formToValidate.clear();
}

void Headmaster::receiveForm(Form* p_form)
{
	if (p_form)
	{
		std::cout << MAGENTA << "Headmaster received form" << RESET << std::endl;
		_formToValidate.push_back(p_form);
		sign(p_form);
		p_form->execute();
	}
}
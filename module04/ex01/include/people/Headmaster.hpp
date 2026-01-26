#ifndef HEADMASTER_HPP
#define HEADMASTER_HPP

#include "Staff.hpp"
#include <vector>

class Form;

class Headmaster : public Staff
{
private:
	std::vector<Form*> _formToValidate;

public:
	Headmaster(std::string p_name);
	void receiveForm(Form* p_form);
};

#endif
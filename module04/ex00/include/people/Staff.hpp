#ifndef STAFF_HPP
#define STAFF_HPP

#include "Person.hpp"

class Form;

class Staff : public Person
{
private:

public:
	void sign(Form* p_form);
};

#endif
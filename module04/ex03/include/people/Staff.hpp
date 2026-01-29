#ifndef STAFF_HPP
#define STAFF_HPP

#include "Person.hpp"

class Form;

class Staff : public Person
{
private:

public:
	Staff(std::string p_name);
	virtual ~Staff();
	
	void sign(Form* p_form);
};

#endif
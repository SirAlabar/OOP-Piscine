#ifndef FORM_HPP
#define FORM_HPP

#include "../enums/FormType.hpp"

class Form
{
private:
	FormType _formType;

public:
	Form(FormType p_formType);
	virtual void execute() = 0;
	virtual ~Form() {}

	FormType getFormType() const { return _formType; }
};

#endif
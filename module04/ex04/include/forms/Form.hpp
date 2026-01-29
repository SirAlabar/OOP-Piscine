#ifndef FORM_HPP
#define FORM_HPP

#include "enums/FormType.hpp"

class Form
{
private:
	FormType _formType;

protected:
	bool _signed;

public:
	Form(FormType p_formType);
	
	virtual void execute() = 0;
	virtual ~Form() {}
	
	void sign();
	bool isSigned() const { return _signed; }
	FormType getFormType() const { return _formType; }
};

#endif
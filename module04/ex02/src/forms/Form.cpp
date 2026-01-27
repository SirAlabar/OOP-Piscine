#include "forms/Form.hpp"

Form::Form(FormType p_formType) : _formType(p_formType), _signed(false)
{
}

void Form::sign()
{
	_signed = true;
}
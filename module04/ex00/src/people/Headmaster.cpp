#include "people/Headmaster.hpp"
#include "forms/Form.hpp"

void Headmaster::receiveForm(Form* p_form)
{
	if (p_form)
		_formToValidate.push_back(p_form);
}
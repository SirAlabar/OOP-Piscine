#include "people/Headmaster.hpp"
#include "forms/Form.hpp"

Headmaster::Headmaster(std::string p_name) : Staff(p_name)
{
}

void Headmaster::receiveForm(Form* p_form)
{
	if (p_form)
		_formToValidate.push_back(p_form);
}
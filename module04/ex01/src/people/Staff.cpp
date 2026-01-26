#include "people/Staff.hpp"
#include "forms/Form.hpp"

Staff::Staff(std::string p_name) : Person(p_name)
{
}

void Staff::sign(Form* p_form)
{
	if (p_form)
	{
		// Sign implementation
	}
}
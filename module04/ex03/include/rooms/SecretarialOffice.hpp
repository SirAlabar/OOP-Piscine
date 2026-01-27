#ifndef SECRETARIALOFFICE_HPP
#define SECRETARIALOFFICE_HPP

#include "Room.hpp"
#include <vector>

class Form;

class SecretarialOffice : public Room
{
private:
	std::vector<Form*> _archivedForms;

public:

};

#endif
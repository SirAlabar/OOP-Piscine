#include "rooms/Room.hpp"
#include "people/Person.hpp"
#include <iostream>

long long Room::_nextID = 1;

Room::Room() : ID(_nextID++)
{
}

bool Room::canEnter(Person* p_person)
{
	return (p_person != NULL);
}

void Room::enter(Person* p_person)
{
	if (canEnter(p_person))
		_occupants.push_back(p_person);
}

void Room::exit(Person* p_person)
{
	for (std::vector<Person*>::iterator it = _occupants.begin(); it != _occupants.end(); ++it)
	{
		if (*it == p_person)
		{
			_occupants.erase(it);
			return;
		}
	}
}

void Room::printOccupant()
{
	std::cout << "Room " << ID << " occupants:" << std::endl;
	for (std::vector<Person*>::iterator it = _occupants.begin(); it != _occupants.end(); ++it)
	{
		std::cout << "  - " << (*it)->getName() << std::endl;
	}
}
#ifndef PERSON_HPP
#define PERSON_HPP

#include <string>

class Room;

class Person
{
private:
	std::string _name;
	Room* _currentRoom;

public:
	Room* room() { return (_currentRoom); }
	const std::string& getName() const { return _name; }
};

#endif
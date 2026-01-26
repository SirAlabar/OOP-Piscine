#ifndef ROOM_HPP
#define ROOM_HPP

#include <vector>
#include <string>
#include <sstream>

class Person;

class Room
{
private:
	long long ID;
	std::vector<Person*> _occupants;
	static long long _nextID;

public:
	Room();
	bool canEnter(Person*);
	void enter(Person*);
	void exit(Person*);
	void printOccupant();
	
	long long getID() const { return ID; }
	
	std::string getName() const
	{
		std::stringstream ss;
		ss << "Room_" << ID;
		return ss.str();
	}
};

#endif
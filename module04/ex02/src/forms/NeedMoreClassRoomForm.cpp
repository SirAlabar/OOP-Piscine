#include "forms/NeedMoreClassRoomForm.hpp"
#include "academic/Classroom.hpp"
#include "rooms/Room.hpp"
#include "patterns/singetons.hpp"
#include <iostream>

#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"

NeedMoreClassRoomForm::NeedMoreClassRoomForm() : 
	Form(NeedMoreClassRoom),
	_createdClassroom(NULL)
{
}

void NeedMoreClassRoomForm::execute()
{
	if (!_signed)
	{
		std::cout << RED << "✗ NeedMoreClassRoomForm not signed! Cannot execute." << RESET << std::endl;
		return;
	}
	
	_createdClassroom = new Classroom();
	
	RoomList::getInstance().add(_createdClassroom);
	
	std::cout << GREEN << "✓ New classroom created: " << _createdClassroom->getName() << RESET << std::endl;
}
#include "academic/Classroom.hpp"
#include "academic/Course.hpp"

Classroom::Classroom() : Room(), _currentRoom(NULL)
{
}

void Classroom::assignCourse(Course* p_course)
{
	_currentRoom = p_course;
}
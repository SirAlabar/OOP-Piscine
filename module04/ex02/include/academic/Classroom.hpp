#ifndef CLASSROOM_HPP
#define CLASSROOM_HPP

#include "rooms/Room.hpp"

class Course;

class Classroom : public Room
{
private:
	Course* _currentRoom;

public:
	Classroom();
	void assignCourse(Course* p_course);
};

#endif
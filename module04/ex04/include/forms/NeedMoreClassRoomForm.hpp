#ifndef NEEDMORECLASSROOMFORM_HPP
#define NEEDMORECLASSROOMFORM_HPP

#include "Form.hpp"

class Classroom;

class NeedMoreClassRoomForm : public Form
{
private:
	Classroom* _createdClassroom;

public:
	NeedMoreClassRoomForm();
	
	Classroom* getCreatedClassroom() const { return _createdClassroom; }
	
	void execute();
};

#endif
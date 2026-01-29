#include "people/Professor.hpp"
#include "people/Headmaster.hpp"
#include "people/Student.hpp"
#include "academic/Course.hpp"
#include "academic/Classroom.hpp"
#include "patterns/singetons.hpp"
#include <iostream>

#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define RESET "\033[0m"

Professor::Professor(std::string p_name) : Staff(p_name), _currentCourse(NULL), _onBreak(false)
{
}

Professor::~Professor()
{
}

void Professor::assignCourse(Course* p_course)
{
	_currentCourse = p_course;
	std::cout << GREEN << getName() << " assigned to course: " << p_course->getName() << RESET << std::endl;
}

void Professor::doClass(Headmaster* headmaster)
{
	if (!_currentCourse)
	{
		std::cout << YELLOW << getName() << " has no course to teach!" << RESET << std::endl;
		std::cout << getName() << ": I need a course!" << std::endl;
		return;
	}
	
	Classroom* classroom = findAvailableClassroom();
	if (!classroom)
	{
		std::cout << YELLOW << getName() << ": No free classroom available!" << RESET << std::endl;
		std::cout << getName() << ": Headmaster, I need a classroom!" << std::endl;
		headmaster->processClassroomRequest();
		return;
	}
	
	std::cout << GREEN << getName() << " is teaching " << _currentCourse->getName() 
	          << " in " << classroom->getName() << RESET << std::endl;
	
	classroom->startClass(this, _currentCourse);
	
	const std::vector<Student*>& students = _currentCourse->getStudents();
	for (std::vector<Student*>::const_iterator it = students.begin(); it != students.end(); ++it)
	{
		_currentCourse->markAttendance(*it);
		std::cout << "  - " << (*it)->getName() << " attended (total: " 
		          << _currentCourse->getAttendance(*it) << "/" 
		          << _currentCourse->getRequiredClasses() << ")" << std::endl;
	}
	
	classroom->endClass();
}

void Professor::closeCourse()
{
	if (_currentCourse)
	{
		std::cout << GREEN << getName() << " closed course: " << _currentCourse->getName() << RESET << std::endl;
	}
}

void Professor::requestCourseCreation(Headmaster* headmaster, std::string courseName)
{
	std::cout << getName() << ": I need a course called '" << courseName << "'" << std::endl;
	headmaster->processCourseCreationRequest(this, courseName);
}

void Professor::requestStudentGraduation(Headmaster* headmaster, Student* student, Course* course)
{
	std::cout << getName() << ": Student " << student->getName() 
	          << " is ready to graduate from " << course->getName() << std::endl;
	headmaster->processGraduationRequest(this, student, course);
}

Classroom* Professor::findAvailableClassroom()
{
	std::vector<Room*>& rooms = RoomList::getInstance().getAll();
	
	for (std::vector<Room*>::iterator it = rooms.begin(); it != rooms.end(); ++it)
	{
		Classroom* classroom = dynamic_cast<Classroom*>(*it);
		if (classroom && classroom->isAvailable())
		{
			return classroom;
		}
	}
	
	return NULL;
}

void Professor::update(Event event)
{
	if (event == RingBell)
	{
		_onBreak = !_onBreak;
		if (_onBreak)
		{
			std::cout << GREEN << getName() << ": Finally, a break! ☕" << RESET << std::endl;
		}
		else
		{
			std::cout << GREEN << getName() << ": Break over, time to teach!" << RESET << std::endl;
		}
	}
	else if (event == GraduationCeremony)
	{
		std::cout << GREEN << getName() << ": 🎓 Proud of all our graduates!" << RESET << std::endl;
	}
}
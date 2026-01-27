#include "people/Professor.hpp"
#include "people/Headmaster.hpp"
#include "people/Student.hpp"
#include "academic/Course.hpp"
#include <iostream>

#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

Professor::Professor(std::string p_name) : Staff(p_name), _currentCourse(NULL)
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

void Professor::doClass()
{
	if (!_currentCourse)
	{
		std::cout << YELLOW << getName() << " has no course to teach!" << RESET << std::endl;
		return;
	}
	
	std::cout << GREEN << getName() << " is teaching " << _currentCourse->getName() << RESET << std::endl;
	
	const std::vector<Student*>& students = _currentCourse->getStudents();
	for (std::vector<Student*>::const_iterator it = students.begin(); it != students.end(); ++it)
	{
		_currentCourse->markAttendance(*it);
		std::cout << "  - " << (*it)->getName() << " attended (total: " 
		          << _currentCourse->getAttendance(*it) << ")" << std::endl;
	}
}

void Professor::closeCourse()
{
	if (_currentCourse)
		std::cout << GREEN << getName() << " closed course: " << _currentCourse->getName() << RESET << std::endl;
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
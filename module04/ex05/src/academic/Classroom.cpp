#include "academic/Classroom.hpp"
#include "academic/Course.hpp"
#include "people/Professor.hpp"
#include "people/Student.hpp"

Classroom::Classroom() : Room(), _currentCourse(NULL), _currentProfessor(NULL), _occupied(false)
{
}

void Classroom::assignCourse(Course* p_course)
{
	_currentCourse = p_course;
}

void Classroom::startClass(Professor* professor, Course* course)
{
	_occupied = true;
	_currentProfessor = professor;
	_currentCourse = course;
	
	enter(professor);
	
	const std::vector<Student*>& students = course->getStudents();
	for (std::vector<Student*>::const_iterator it = students.begin(); it != students.end(); ++it)
	{
		_attendingStudents.push_back(*it);
		enter(*it);
	}
}

void Classroom::endClass()
{
	if (_currentProfessor)
		exit(_currentProfessor);
	
	for (std::vector<Student*>::iterator it = _attendingStudents.begin(); it != _attendingStudents.end(); ++it)
	{
		exit(*it);
	}
	
	_attendingStudents.clear();
	_currentProfessor = NULL;
	_currentCourse = NULL;
	_occupied = false;
}
#include "academic/Course.hpp"
#include "people/Professor.hpp"
#include "people/Student.hpp"

Course::Course(std::string p_name) : 
	_name(p_name), 
	_responsable(NULL),
	_numberOfClassToGraduate(0),
	_maximumNumberOfStudent(0)
{
}

void Course::assign(Professor* p_professor)
{
	_responsable = p_professor;
}

void Course::subscribe(Student* p_student)
{
	if (p_student)
		_students.push_back(p_student);
}
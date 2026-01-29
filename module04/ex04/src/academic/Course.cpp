#include "academic/Course.hpp"
#include "people/Professor.hpp"
#include "people/Student.hpp"
#include <algorithm>

Course::Course(std::string p_name) : 
	_name(p_name), 
	_responsable(NULL),
	_numberOfClassToGraduate(5),
	_maximumNumberOfStudent(30)
{
}

void Course::assign(Professor* p_professor)
{
	_responsable = p_professor;
}

void Course::subscribe(Student* p_student)
{
	if (p_student && _students.size() < static_cast<size_t>(_maximumNumberOfStudent))
	{
		_students.push_back(p_student);
		_attendanceCount[p_student] = 0; 
	}
}

void Course::unsubscribe(Student* p_student)
{
	std::vector<Student*>::iterator it = std::find(_students.begin(), _students.end(), p_student);
	if (it != _students.end())
	{
		_students.erase(it);
		_attendanceCount.erase(p_student);
	}
}

void Course::markAttendance(Student* p_student)
{
	if (_attendanceCount.find(p_student) != _attendanceCount.end())
		_attendanceCount[p_student]++;
}

bool Course::canGraduate(Student* p_student) const
{
	std::map<Student*, int>::const_iterator it = _attendanceCount.find(p_student);
	if (it == _attendanceCount.end())
		return false;
	return it->second >= _numberOfClassToGraduate;
}

int Course::getAttendance(Student* p_student) const
{
	std::map<Student*, int>::const_iterator it = _attendanceCount.find(p_student);
	if (it == _attendanceCount.end())
		return 0;
	return it->second;
}
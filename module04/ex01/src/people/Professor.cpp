#include "people/Professor.hpp"
#include "academic/Course.hpp"

Professor::Professor(std::string p_name) : Staff(p_name), _currentCourse(NULL)
{
}

void Professor::assignCourse(Course* p_course)
{
	_currentCourse = p_course;
}

void Professor::doClass()
{
}

void Professor::closeCourse()
{
}
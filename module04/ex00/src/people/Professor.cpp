#include "people/Professor.hpp"
#include "academic/Course.hpp"

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
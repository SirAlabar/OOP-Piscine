#ifndef PROFESSOR_HPP
#define PROFESSOR_HPP

#include "Staff.hpp"

class Course;

class Professor : public Staff
{
private:
	Course* _currentCourse;

public:
	Professor(std::string p_name);
	void assignCourse(Course* p_course);
	void doClass();
	void closeCourse();
};

#endif
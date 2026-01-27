#ifndef PROFESSOR_HPP
#define PROFESSOR_HPP

#include "Staff.hpp"

class Course;
class Headmaster;
class Student;

class Professor : public Staff
{
private:
	Course* _currentCourse;

public:
	Professor(std::string p_name);
	~Professor();
	
	void assignCourse(Course* p_course);
	void doClass();
	void closeCourse();
	
	void requestCourseCreation(Headmaster* headmaster, std::string courseName);
	void requestStudentGraduation(Headmaster* headmaster, Student* student, Course* course);
	
	Course* getCurrentCourse() const { return _currentCourse; }
};

#endif
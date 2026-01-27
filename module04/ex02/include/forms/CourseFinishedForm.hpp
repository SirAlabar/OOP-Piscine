#ifndef COURSEFINISHEDFORM_HPP
#define COURSEFINISHEDFORM_HPP

#include "Form.hpp"

class Student;
class Course;

class CourseFinishedForm : public Form
{
private:
	Student* _student;
	Course* _course;

public:
	CourseFinishedForm();
	
	void setStudent(Student* p_student) { _student = p_student; }
	void setCourse(Course* p_course) { _course = p_course; }
	
	void execute();
};

#endif
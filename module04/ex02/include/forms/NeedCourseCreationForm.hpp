#ifndef NEEDCOURSECREATIONFORM_HPP
#define NEEDCOURSECREATIONFORM_HPP

#include "Form.hpp"
#include <string>

class Professor;
class Course;

class NeedCourseCreationForm : public Form
{
private:
	std::string _courseName;
	Professor* _professor;
	Course* _createdCourse;

public:
	NeedCourseCreationForm();
	
	void setCourseName(std::string p_name) { _courseName = p_name; }
	void setProfessor(Professor* p_prof) { _professor = p_prof; }
	Course* getCreatedCourse() const { return _createdCourse; }
	
	void execute();
};

#endif
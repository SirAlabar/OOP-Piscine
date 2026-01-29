#ifndef HEADMASTER_HPP
#define HEADMASTER_HPP

#include "Staff.hpp"
#include <vector>

class Form;
class Secretary;
class Student;
class Professor;
class Course;
class Bell;

class Headmaster : public Staff
{
private:
	Secretary* _secretary;
	Bell* _bell;
	std::vector<Form*> _formToValidate;

public:
	Headmaster(std::string p_name, Secretary* p_secretary);
	~Headmaster();
	
	void processCourseCreationRequest(Professor* professor, std::string courseName);
	void processSubscriptionRequest(Student* student, Course* course);
	void processGraduationRequest(Professor* professor, Student* student, Course* course);
	void processClassroomRequest();
	
	void launchClasses();
	void requestRingBell();
	
	void receiveForm(Form* p_form);
	
	Bell* getBell() const { return _bell; }
};

#endif
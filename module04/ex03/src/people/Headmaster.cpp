#include "people/Headmaster.hpp"
#include "people/Secretary.hpp"
#include "people/Professor.hpp"
#include "people/Student.hpp"
#include "academic/Course.hpp"
#include "forms/Form.hpp"
#include "forms/NeedCourseCreationForm.hpp"
#include "forms/SubscriptionToCourseForm.hpp"
#include "forms/CourseFinishedForm.hpp"
#include "forms/NeedMoreClassRoomForm.hpp"
#include <iostream>

#define MAGENTA "\033[35m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define RESET "\033[0m"

Headmaster::Headmaster(std::string p_name, Secretary* p_secretary) : 
	Staff(p_name), 
	_secretary(p_secretary)
{
	std::cout << MAGENTA << "Headmaster " << p_name << " initialized with secretary" << RESET << std::endl;
}

Headmaster::~Headmaster()
{
	for (std::vector<Form*>::iterator it = _formToValidate.begin(); it != _formToValidate.end(); ++it)
	{
		delete *it;
	}
	_formToValidate.clear();
}

void Headmaster::processCourseCreationRequest(Professor* professor, std::string courseName)
{
	std::cout << MAGENTA << "Headmaster: Processing course creation request from " 
	          << professor->getName() << RESET << std::endl;
	
	Form* form = _secretary->createForm(NeedCourseCreation);
	
	NeedCourseCreationForm* courseForm = static_cast<NeedCourseCreationForm*>(form);
	courseForm->setCourseName(courseName);
	courseForm->setProfessor(professor);
	
	std::cout << MAGENTA << "Headmaster: Form prepared, signing and executing..." << RESET << std::endl;
	
	receiveForm(form);
}

void Headmaster::processSubscriptionRequest(Student* student, Course* course)
{
	std::cout << MAGENTA << "Headmaster: Processing subscription request from " 
	          << student->getName() << RESET << std::endl;
	
	if (student->isSubscribedTo(course))
	{
		std::cout << YELLOW << "Headmaster: Student already subscribed to this course" << RESET << std::endl;
		return;
	}
	
	Form* form = _secretary->createForm(SubscriptionToCourse);
	
	SubscriptionToCourseForm* subForm = static_cast<SubscriptionToCourseForm*>(form);
	subForm->setStudent(student);
	subForm->setCourse(course);
	
	std::cout << MAGENTA << "Headmaster: Form prepared, signing and executing..." << RESET << std::endl;
	
	receiveForm(form);
}

void Headmaster::processGraduationRequest(Professor* professor, Student* student, Course* course)
{
	std::cout << MAGENTA << "Headmaster: Processing graduation request from " 
	          << professor->getName() << RESET << std::endl;
	
	if (!course->canGraduate(student))
	{
		std::cout << YELLOW << "Headmaster: Student has not completed requirements. Attendance: " 
		          << course->getAttendance(student) << "/" << course->getRequiredClasses() 
		          << RESET << std::endl;
		return;
	}

	Form* form = _secretary->createForm(CourseFinished);
	
	CourseFinishedForm* gradForm = static_cast<CourseFinishedForm*>(form);
	gradForm->setStudent(student);
	gradForm->setCourse(course);
	
	std::cout << MAGENTA << "Headmaster: Graduation form prepared, signing and executing..." << RESET << std::endl;
	
	receiveForm(form);
}

void Headmaster::processClassroomRequest()
{
	std::cout << MAGENTA << "Headmaster: Processing classroom creation request" << RESET << std::endl;
	
	Form* form = _secretary->createForm(NeedMoreClassRoom);
	
	std::cout << MAGENTA << "Headmaster: Classroom form prepared, signing and executing..." << RESET << std::endl;
	
	receiveForm(form);
}

void Headmaster::receiveForm(Form* p_form)
{
	if (p_form)
	{
		std::cout << MAGENTA << "Headmaster received form" << RESET << std::endl;
		_formToValidate.push_back(p_form);
		sign(p_form);
		p_form->execute();
	}
}

void Headmaster::launchClasses()
{
	std::cout << CYAN << "Headmaster: Launching classes! Professors, attend your classes!" << RESET << std::endl;
}

void Headmaster::requestRingBell()
{
	std::cout << CYAN << "Headmaster: *RING BELL* Break time!" << RESET << std::endl;
}

#include "forms/NeedCourseCreationForm.hpp"
#include "people/Professor.hpp"
#include "academic/Course.hpp"
#include "patterns/singetons.hpp"
#include <iostream>

#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"

NeedCourseCreationForm::NeedCourseCreationForm() : 
	Form(NeedCourseCreation), 
	_courseName(""), 
	_professor(NULL),
	_createdCourse(NULL)
{
}

void NeedCourseCreationForm::execute()
{
	if (!_signed)
	{
		std::cout << RED << "✗ NeedCourseCreationForm not signed! Cannot execute." << RESET << std::endl;
		return;
	}
	
	if (_courseName.empty() || !_professor)
	{
		std::cout << RED << "✗ NeedCourseCreationForm incomplete! Missing course name or professor." << RESET << std::endl;
		return;
	}
	
	_createdCourse = new Course(_courseName);
	_createdCourse->assign(_professor);
	_professor->assignCourse(_createdCourse);
	
	CourseList::getInstance().add(_createdCourse);
	
	std::cout << GREEN << "✓ Course '" << _courseName << "' created and assigned to " << _professor->getName() << RESET << std::endl;
}
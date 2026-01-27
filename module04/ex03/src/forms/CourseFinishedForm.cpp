#include "forms/CourseFinishedForm.hpp"
#include "people/Student.hpp"
#include "academic/Course.hpp"
#include <iostream>

#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"

CourseFinishedForm::CourseFinishedForm() : Form(CourseFinished), _student(NULL), _course(NULL)
{
}

void CourseFinishedForm::execute()
{
	if (!_signed)
	{
		std::cout << RED << "✗ CourseFinishedForm not signed! Cannot execute." << RESET << std::endl;
		return;
	}
	
	if (!_student || !_course)
	{
		std::cout << RED << "✗ CourseFinishedForm incomplete! Missing student or course." << RESET << std::endl;
		return;
	}
	
	_student->graduate(_course);
	std::cout << GREEN << "✓ " << _student->getName() << " graduated from " << _course->getName() << RESET << std::endl;
}
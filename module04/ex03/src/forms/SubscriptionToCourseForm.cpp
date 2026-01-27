#include "forms/SubscriptionToCourseForm.hpp"
#include "people/Student.hpp"
#include "academic/Course.hpp"
#include <iostream>

#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"

SubscriptionToCourseForm::SubscriptionToCourseForm() : 
	Form(SubscriptionToCourse),
	_student(NULL),
	_course(NULL)
{
}

void SubscriptionToCourseForm::execute()
{
	if (!_signed)
	{
		std::cout << RED << "✗ SubscriptionToCourseForm not signed! Cannot execute." << RESET << std::endl;
		return;
	}
	
	if (!_student || !_course)
	{
		std::cout << RED << "✗ SubscriptionToCourseForm incomplete! Missing student or course." << RESET << std::endl;
		return;
	}
	
	_course->subscribe(_student);
	std::cout << GREEN << "✓ " << _student->getName() << " subscribed to " << _course->getName() << RESET << std::endl;
}
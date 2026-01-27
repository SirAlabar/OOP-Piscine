#include "people/Secretary.hpp"
#include "forms/Form.hpp"
#include "forms/CourseFinishedForm.hpp"
#include "forms/NeedMoreClassRoomForm.hpp"
#include "forms/NeedCourseCreationForm.hpp"
#include "forms/SubscriptionToCourseForm.hpp"
#include <iostream>

#define CYAN "\033[36m"
#define RESET "\033[0m"

Secretary::Secretary(std::string p_name) : Staff(p_name)
{
}

//Factory
Form* Secretary::createForm(FormType p_formType)
{
	Form* form = NULL;
	
	switch (p_formType)
	{
		case CourseFinished:
			form = new CourseFinishedForm();
			std::cout << CYAN << "Secretary created CourseFinishedForm" << RESET << std::endl;
			break;
		case NeedCourseCreation:
			form = new NeedCourseCreationForm();
			std::cout << CYAN << "Secretary created NeedCourseCreationForm" << RESET << std::endl;
			break;
		case NeedMoreClassRoom:
			form = new NeedMoreClassRoomForm();
			std::cout << CYAN << "Secretary created NeedMoreClassRoomForm" << RESET << std::endl;
			break;
		case SubscriptionToCourse:
			form = new SubscriptionToCourseForm();
			std::cout << CYAN << "Secretary created SubscriptionToCourseForm" << RESET << std::endl;
			break;
	}
	
	return form;
}

void Secretary::archiveForm()
{
	// Implementation for archiving forms
}
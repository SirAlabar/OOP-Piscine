#include "people/Student.hpp"
#include "people/Headmaster.hpp"
#include "academic/Classroom.hpp"
#include "academic/Course.hpp"
#include <iostream>
#include <algorithm>

#define BLUE "\033[34m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

Student::Student(std::string p_name) : Person(p_name), _onBreak(false)
{
}

Student::~Student()
{
}

void Student::attendClass(Classroom* p_classroom)
{
	if (p_classroom)
	{
		std::cout << BLUE << getName() << " attending class" << RESET << std::endl;
	}
}

void Student::exitClass()
{
	std::cout << BLUE << getName() << " exiting class" << RESET << std::endl;
}

void Student::graduate(Course* p_course)
{
	if (p_course)
	{
		std::vector<Course*>::iterator it = std::find(_subscribedCourse.begin(), _subscribedCourse.end(), p_course);
		if (it != _subscribedCourse.end())
		{
			_subscribedCourse.erase(it);
		}
		
		std::cout << BLUE << getName() << " GRADUATED from " << p_course->getName() << "! 🎓" << RESET << std::endl;
	}
}

void Student::requestCourseSubscription(Headmaster* headmaster, Course* course)
{
	std::cout << getName() << ": I want to join " << course->getName() << std::endl;
	headmaster->processSubscriptionRequest(this, course);
}

void Student::update(Event event)
{
	if (event == RingBell)
	{
		_onBreak = !_onBreak;
		if (_onBreak)
		{
			std::cout << GREEN << getName() << ": Yay! Break time! 🎉" << RESET << std::endl;
		}
		else
		{
			std::cout << BLUE << getName() << ": Break is over, back to class!" << RESET << std::endl;
		}
	}
}

bool Student::isSubscribedTo(Course* course) const
{
	return std::find(_subscribedCourse.begin(), _subscribedCourse.end(), course) != _subscribedCourse.end();
}
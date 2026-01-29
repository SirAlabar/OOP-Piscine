#include <iostream>
#include "patterns/singetons.hpp"
#include "people/Professor.hpp"
#include "people/Secretary.hpp"
#include "people/Headmaster.hpp"
#include "people/Student.hpp"
#include "academic/Course.hpp"

#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"

int main()
{
	std::cout << CYAN << "=== Ex03: Mediator Pattern ===" << RESET << std::endl << std::endl;

	Secretary* secretary = new Secretary("Secretary Mary");
	Headmaster* headmaster = new Headmaster("Headmaster Constantine", secretary);
	
	Professor* profSmith = new Professor("Prof. Smith");
	Professor* profJones = new Professor("Prof. Jones");
	Student* alice = new Student("Alice");
	Student* bob = new Student("Bob");
	
	StaffList::getInstance().add(secretary);
	StaffList::getInstance().add(headmaster);
	StaffList::getInstance().add(profSmith);
	StaffList::getInstance().add(profJones);
	StudentList::getInstance().add(alice);
	StudentList::getInstance().add(bob);
	
	std::cout << std::endl;

	std::cout << BLUE << "=== Scenario 1: Professors Have No Course ===" << RESET << std::endl;
	headmaster->launchClasses();
	std::cout << std::endl;

	std::cout << BLUE << "=== Scenario 2: Professor Requests Course Creation ===" << RESET << std::endl;
	if (!profSmith->getCurrentCourse())
	{
		profSmith->requestCourseCreation(headmaster, "Mathematics");
	}
	if (!profJones->getCurrentCourse())
	{
		profJones->requestCourseCreation(headmaster, "Physics");
	}
	std::cout << std::endl;

	std::cout << BLUE << "=== Scenario 3: Students Request Course Subscription ===" << RESET << std::endl;
	Course* math = profSmith->getCurrentCourse();
	if (math)
	{
		alice->requestCourseSubscription(headmaster, math);
		bob->requestCourseSubscription(headmaster, math);
	}
	std::cout << std::endl;

	std::cout << BLUE << "=== Scenario 4: Professor Needs Classroom (None Available) ===" << RESET << std::endl;
	headmaster->launchClasses();
	std::cout << std::endl;

	std::cout << BLUE << "=== Scenario 5: Classes Begin With Classroom ===" << RESET << std::endl;
	headmaster->launchClasses();
	std::cout << std::endl;
	
	headmaster->launchClasses();
	std::cout << std::endl;
	
	headmaster->launchClasses();
	std::cout << std::endl;
	
	headmaster->launchClasses();
	std::cout << std::endl;
	
	headmaster->launchClasses();
	std::cout << std::endl;

	std::cout << BLUE << "=== Scenario 6: Professor Recommends Graduation ===" << RESET << std::endl;
	
	if (math->canGraduate(alice))
	{
		profSmith->requestStudentGraduation(headmaster, alice, math);
	}
	
	if (math->canGraduate(bob))
	{
		profSmith->requestStudentGraduation(headmaster, bob, math);
	}
	std::cout << std::endl;

	std::cout << BLUE << "=== Scenario 7: Attempt Early Graduation (Should Fail) ===" << RESET << std::endl;
	Student* charlie = new Student("Charlie");
	StudentList::getInstance().add(charlie);
	charlie->requestCourseSubscription(headmaster, math);
	profSmith->requestStudentGraduation(headmaster, charlie, math);
	std::cout << std::endl;

	std::cout << CYAN << "=== Ex03 Complete ===" << RESET << std::endl;
	std::cout << std::endl;
	
	std::cout << GREEN << "✓ Complete! (All memory auto-cleanup via Singletons)" << RESET << std::endl;

	return 0;
}
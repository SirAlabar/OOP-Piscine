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
	
	StaffList::getInstance().add(static_cast<Staff*>(profSmith));
	StaffList::getInstance().add(static_cast<Staff*>(profJones));
	
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
	charlie->requestCourseSubscription(headmaster, math);
	profSmith->requestStudentGraduation(headmaster, charlie, math);
	std::cout << std::endl;

	std::cout << CYAN << "=== Ex03 Complete ===" << RESET << std::endl;
	std::cout << std::endl;

	std::cout << GREEN << "Cleaning up memory..." << RESET << std::endl;
	
	delete charlie;
	delete alice;
	delete bob;
	delete profSmith;
	delete profJones;
	delete headmaster;
	delete secretary;
	
	std::vector<Course*>& courses = CourseList::getInstance().getAll();
	for (std::vector<Course*>::iterator it = courses.begin(); it != courses.end(); ++it)
	{
		delete *it;
	}
	
	std::vector<Room*>& rooms = RoomList::getInstance().getAll();
	for (std::vector<Room*>::iterator it = rooms.begin(); it != rooms.end(); ++it)
	{
		delete *it;
	}
	
	std::cout << GREEN << "✓ Cleanup complete!" << RESET << std::endl;

	return 0;
}
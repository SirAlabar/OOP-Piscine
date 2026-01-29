#include <iostream>
#include "patterns/singetons.hpp"
#include "people/Professor.hpp"
#include "people/Secretary.hpp"
#include "people/Headmaster.hpp"
#include "people/Student.hpp"
#include "academic/Course.hpp"
#include "events/Bell.hpp"

#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define YELLOW  "\033[33m"

int main()
{
	std::cout << CYAN << "=== Ex04: Observer Pattern - Bell System ===" << RESET << std::endl << std::endl;

	Secretary* secretary = new Secretary("Secretary Mary");
	Headmaster* headmaster = new Headmaster("Headmaster Constantine", secretary);
	Professor* profSmith = new Professor("Prof. Smith");
	Student* alice = new Student("Alice");
	Student* bob = new Student("Bob");
	
	// Add EVERYONE to singletons
	StaffList::getInstance().add(secretary);
	StaffList::getInstance().add(headmaster);
	StaffList::getInstance().add(profSmith);
	StudentList::getInstance().add(alice);
	StudentList::getInstance().add(bob);
	
	std::cout << BLUE << "=== Subscribe to Bell System ===" << RESET << std::endl;
	Bell* bell = headmaster->getBell();
	bell->subscribe(profSmith);
	bell->subscribe(alice);
	bell->subscribe(bob);
	std::cout << GREEN << "✓ Professor and students subscribed to bell" << RESET << std::endl;
	std::cout << std::endl;

	std::cout << BLUE << "=== Setup Course ===" << RESET << std::endl;
	profSmith->requestCourseCreation(headmaster, "Mathematics");
	Course* math = profSmith->getCurrentCourse();
	alice->requestCourseSubscription(headmaster, math);
	bob->requestCourseSubscription(headmaster, math);
	std::cout << std::endl;

	std::cout << YELLOW << "========== SCHOOL DAY BEGINS ==========" << RESET << std::endl;
	std::cout << std::endl;

	std::cout << BLUE << "--- Morning Classes ---" << RESET << std::endl;
	headmaster->launchClasses();
	std::cout << std::endl;
	headmaster->launchClasses();
	std::cout << std::endl;

	std::cout << YELLOW << "--- Morning Break (Bell rings) ---" << RESET << std::endl;
	headmaster->requestRingBell();
	std::cout << std::endl;

	std::cout << YELLOW << "--- Break Over (Bell rings) ---" << RESET << std::endl;
	headmaster->requestRingBell();
	std::cout << std::endl;

	std::cout << BLUE << "--- Late Morning Classes ---" << RESET << std::endl;
	headmaster->launchClasses();
	std::cout << std::endl;

	std::cout << YELLOW << "--- Lunch Time (Bell rings) ---" << RESET << std::endl;
	headmaster->requestRingBell();
	std::cout << std::endl;

	std::cout << YELLOW << "--- After Lunch (Bell rings) ---" << RESET << std::endl;
	headmaster->requestRingBell();
	std::cout << std::endl;

	std::cout << BLUE << "--- Afternoon Classes ---" << RESET << std::endl;
	headmaster->launchClasses();
	std::cout << std::endl;
	headmaster->launchClasses();
	std::cout << std::endl;

	std::cout << YELLOW << "--- End of Day (Bell rings) ---" << RESET << std::endl;
	headmaster->requestRingBell();
	std::cout << std::endl;

	std::cout << YELLOW << "========== SCHOOL DAY ENDS ==========" << RESET << std::endl;
	std::cout << std::endl;

	std::cout << GREEN << "✓ Complete! (All memory auto-cleanup via Singletons)" << RESET << std::endl;

	return 0;
}
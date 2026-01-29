#include <iostream>
#include "School.hpp"

#define RESET "\033[0m"
#define CYAN  "\033[36m"

int main()
{
	std::cout << CYAN << "=== Ex05: Facade Pattern - School ===" << RESET << std::endl;
	std::cout << std::endl;

	School* school = new School();
	
	school->recruteProfessor("Prof. Smith");
	school->recruteStudent("Alice");
	school->recruteStudent("Bob");
	
	school->setupSchool();

	school->runDayRoutine();
	// school->runDayRoutine();
	
	school->graduationCeremony();
	
	delete school;
	
	return 0;
}
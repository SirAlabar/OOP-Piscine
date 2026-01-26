#include <iostream>
#include "patterns/singetons.hpp"
#include "people/Professor.hpp"
#include "people/Secretary.hpp"
#include "people/Headmaster.hpp"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

int main()
{
	std::cout << CYAN << "=== Ex01: Singleton Pattern ===" << RESET << std::endl << std::endl;

	// === STUDENTS ===
	std::cout << YELLOW << "Creating students..." << RESET << std::endl;
	Student* alice = new Student("Alice");
	Student* bob = new Student("Bob");
	Student* charlie = new Student("Charlie");

	StudentList::getInstance().add(alice);
	StudentList::getInstance().add(bob);
	StudentList::getInstance().add(charlie);

	std::cout << GREEN << "Students added: " << StudentList::getInstance().size() << RESET << std::endl;

	Student* found = StudentList::getInstance().find("Bob");
	if (found)
		std::cout << GREEN << "✓ Found student: " << found->getName() << RESET << std::endl;
	else
		std::cout << RED << "✗ Student not found!" << RESET << std::endl;

	std::vector<Student*>& allStudents = StudentList::getInstance().getAll();
	std::cout << BLUE << "All students (" << allStudents.size() << "):" << RESET << std::endl;
	for (std::vector<Student*>::iterator it = allStudents.begin(); it != allStudents.end(); ++it)
		std::cout << "  - " << (*it)->getName() << std::endl;

	std::cout << std::endl;

		// === STAFF ===
	std::cout << YELLOW << "Creating staff membres..." << RESET << std::endl;
	Professor* profIndian = new Professor("Prof. Indian");
	Professor* profJonas = new Professor("Prof. Jonas");
	Secretary* secMary = new Secretary("Secretary Mary Jane");
	Headmaster* headJohn = new Headmaster("Headmaster Constantine");

	StaffList::getInstance().add(profIndian);
	StaffList::getInstance().add(profJonas);
	StaffList::getInstance().add(secMary);
	StaffList::getInstance().add(headJohn);

	std::cout << GREEN << "Staff members added: " << StaffList::getInstance().size() << RESET << std::endl;

	Staff* foundStaff = StaffList::getInstance().find("Prof. Indian");
	if (foundStaff)
		std::cout << GREEN << "✓ Found staff: " << foundStaff->getName() << RESET << std::endl;

	std::vector<Staff*>& allStaff = StaffList::getInstance().getAll();
	std::cout << BLUE << "All staff (" << allStaff.size() << "):" << RESET << std::endl;
	for (std::vector<Staff*>::iterator it = allStaff.begin(); it != allStaff.end(); ++it)
		std::cout << "  - " << (*it)->getName() << std::endl;

	std::cout << std::endl;

	// === COURSES ===
	std::cout << YELLOW << "Creating courses..." << RESET << std::endl;
	Course* math = new Course("Mathematics");
	Course* physics = new Course("Physics");
	Course* chemistry = new Course("Chemistry");

	CourseList::getInstance().add(math);
	CourseList::getInstance().add(physics);
	CourseList::getInstance().add(chemistry);

	std::cout << GREEN << "Courses added: " << CourseList::getInstance().size() << RESET << std::endl;

	Course* foundCourse = CourseList::getInstance().find("Physics");
	if (foundCourse)
		std::cout << GREEN << "✓ Found course: " << foundCourse->getName() << RESET << std::endl;

	std::vector<Course*>& allCourses = CourseList::getInstance().getAll();
	std::cout << BLUE << "All courses (" << allCourses.size() << "):" << RESET << std::endl;
	for (std::vector<Course*>::iterator it = allCourses.begin(); it != allCourses.end(); ++it)
		std::cout << "  - " << (*it)->getName() << std::endl;

	std::cout << std::endl;

	// === ROOMS ===
	std::cout << YELLOW << "Creating rooms..." << RESET << std::endl;
	Room* room1 = new Room();
	Room* room2 = new Room();
	Room* room3 = new Room();

	RoomList::getInstance().add(room1);
	RoomList::getInstance().add(room2);
	RoomList::getInstance().add(room3);

	std::cout << GREEN << "Rooms added: " << RoomList::getInstance().size() << RESET << std::endl;

	std::vector<Room*>& allRooms = RoomList::getInstance().getAll();
	std::cout << BLUE << "All rooms (" << allRooms.size() << "):" << RESET << std::endl;
	for (std::vector<Room*>::iterator it = allRooms.begin(); it != allRooms.end(); ++it)
		std::cout << "  - " << (*it)->getName() << " (ID: " << (*it)->getID() << ")" << std::endl;

	std::cout << std::endl;

	// === SINGLETON PROPERTY TEST ===
	std::cout << MAGENTA << "=== Singleton Property Test ===" << RESET << std::endl;
	StudentList& list1 = StudentList::getInstance();
	StudentList& list2 = StudentList::getInstance();
	
	std::cout << "list1 address: " << &list1 << std::endl;
	std::cout << "list2 address: " << &list2 << std::endl;
	std::cout << "list1 size: " << list1.size() << std::endl;
	std::cout << "list2 size: " << list2.size() << std::endl;
	
	if (&list1 == &list2)
		std::cout << GREEN << "✓ Same instance - Singleton working!" << RESET << std::endl;
	else
		std::cout << RED << "✗ Different instances - Singleton NOT working!" << RESET << std::endl;

	std::cout << std::endl;
	std::cout << CYAN << "=== Ex01 Complete ===" << RESET << std::endl;

	return 0;
}
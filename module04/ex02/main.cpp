#include <iostream>
#include "patterns/singetons.hpp"
#include "people/Professor.hpp"
#include "people/Secretary.hpp"
#include "people/Headmaster.hpp"
#include "people/Student.hpp"
#include "academic/Course.hpp"
#include "forms/Form.hpp"
#include "forms/CourseFinishedForm.hpp"
#include "forms/NeedCourseCreationForm.hpp"
#include "forms/SubscriptionToCourseForm.hpp"
#include "forms/NeedMoreClassRoomForm.hpp"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

int main()
{
	std::cout << CYAN << "=== Ex02: Factory and Command Pattern ===" << RESET << std::endl << std::endl;

	// Create staff
	Secretary* secretary = new Secretary("Secretary Mary");
	Headmaster* headmaster = new Headmaster("Headmaster John");
	Professor* profSmith = new Professor("Prof. Smith");
	
	// Create students and courses
	Student* alice = new Student("Alice");
	Student* bob = new Student("Bob");
	Course* math = new Course("Mathematics");
	
	std::cout << std::endl;

	// === TEST 1: Course Subscription (SIGNED) ===
	std::cout << BLUE << "=== Test 1: Student Subscription (Signed Form) ===" << RESET << std::endl;
	Form* subForm = secretary->createForm(SubscriptionToCourse);
	SubscriptionToCourseForm* subscribeForm = static_cast<SubscriptionToCourseForm*>(subForm);
	subscribeForm->setStudent(alice);
	subscribeForm->setCourse(math);
	headmaster->receiveForm(subscribeForm);
	std::cout << std::endl;

	// === TEST 2: Unsigned Form Attempt ===
	std::cout << BLUE << "=== Test 2: Unsigned Form (Should Fail) ===" << RESET << std::endl;
	Form* unsignedForm = secretary->createForm(SubscriptionToCourse);
	SubscriptionToCourseForm* unsignedSub = static_cast<SubscriptionToCourseForm*>(unsignedForm);
	unsignedSub->setStudent(bob);
	unsignedSub->setCourse(math);
	std::cout << YELLOW << "Attempting to execute without signature..." << RESET << std::endl;
	unsignedSub->execute();  // Should fail
	std::cout << std::endl;

	// === TEST 3: Course Creation ===
	std::cout << BLUE << "=== Test 3: Course Creation ===" << RESET << std::endl;
	Form* courseForm = secretary->createForm(NeedCourseCreation);
	NeedCourseCreationForm* createCourse = static_cast<NeedCourseCreationForm*>(courseForm);
	createCourse->setCourseName("Physics");
	createCourse->setProfessor(profSmith);
	headmaster->receiveForm(createCourse);
	std::cout << std::endl;

	// === TEST 4: Classroom Creation ===
	std::cout << BLUE << "=== Test 4: Classroom Creation ===" << RESET << std::endl;
	Form* classroomForm = secretary->createForm(NeedMoreClassRoom);
	headmaster->receiveForm(classroomForm);
	std::cout << std::endl;

	// === TEST 5: Graduation ===
	std::cout << BLUE << "=== Test 5: Student Graduation ===" << RESET << std::endl;
	Form* gradForm = secretary->createForm(CourseFinished);
	CourseFinishedForm* graduation = static_cast<CourseFinishedForm*>(gradForm);
	graduation->setStudent(alice);
	graduation->setCourse(math);
	headmaster->receiveForm(graduation);
	std::cout << std::endl;

	// === TEST 6: Incomplete Form ===
	std::cout << BLUE << "=== Test 6: Incomplete Form (Should Fail) ===" << RESET << std::endl;
	Form* incompleteForm = secretary->createForm(CourseFinished);
	headmaster->receiveForm(incompleteForm);  // Missing student and course
	std::cout << std::endl;

	std::cout << CYAN << "=== Ex02 Complete ===" << RESET << std::endl;
	
	delete unsignedForm;
	delete math;
	delete alice;
	delete bob;
	delete profSmith;
	delete headmaster;
	delete secretary;

	return 0;
}
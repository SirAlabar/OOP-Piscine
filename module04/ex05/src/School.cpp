#include "School.hpp"
#include "people/Headmaster.hpp"
#include "people/Secretary.hpp"
#include "people/Professor.hpp"
#include "people/Student.hpp"
#include "academic/Course.hpp"
#include "events/Bell.hpp"
#include "patterns/singetons.hpp"
#include <iostream>

#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"

School::School()
{
	std::cout << CYAN << "=== Initializing School ===" << RESET << std::endl;
	
	_secretary = new Secretary("Secretary");
	_headmaster = new Headmaster("Headmaster", _secretary);
	
	StaffList::getInstance().add(_secretary);
	StaffList::getInstance().add(_headmaster);
	
	std::cout << GREEN << "✓ School initialized with Headmaster and Secretary" << RESET << std::endl;
	std::cout << std::endl;
}

School::~School()
{
	// Singletons handle all cleanup automatically
}

void School::recruteProfessor(std::string name)
{
	Professor* professor = new Professor(name);
	_professors.push_back(professor);
	
	StaffList::getInstance().add(professor);
	_headmaster->getBell()->subscribe(professor);
	
	std::cout << GREEN << "✓ Professor " << name << " recruited" << RESET << std::endl;
}

void School::recruteStudent(std::string name)
{
	Student* student = new Student(name);
	_students.push_back(student);
	
	StudentList::getInstance().add(student);
	_headmaster->getBell()->subscribe(student);
	
	std::cout << GREEN << "✓ Student " << name << " enrolled" << RESET << std::endl;
}

void School::launchClasses()
{
	_headmaster->launchClasses();
}

void School::requestRingBell()
{
	_headmaster->requestRingBell();
}

void School::setupSchool()
{
	std::cout << CYAN << "=== Setting up School System ===" << RESET << std::endl;

	if (_professors.empty() || _students.empty())
	{
		std::cout << YELLOW << "No professors or students to setup!" << RESET << std::endl;
		return;
	}

	//Professor requests course creation
	std::cout << BLUE << "[Setup] Creating course Mathematics" << RESET << std::endl;
	_professors[0]->requestCourseCreation(_headmaster, "Mathematics");

	//Get created course
	Course* math = _professors[0]->getCurrentCourse();
	if (!math)
		math = getCourse("Mathematics");

	if (!math)
	{
		std::cout << MAGENTA << "ERROR: Course was not created!" << RESET << std::endl;
		return;
	}

	std::cout << BLUE << "[Setup] Enrolling students into Mathematics" << RESET << std::endl;
	for (size_t i = 0; i < _students.size(); ++i)
	{
		_students[i]->requestCourseSubscription(_headmaster, math);
	}

	std::cout << GREEN << "✓ School setup completed" << RESET << std::endl << std::endl;
}

void School::runDayRoutine()
{
	std::cout << std::endl;
	std::cout << YELLOW << "========================================" << RESET << std::endl;
	std::cout << YELLOW << "       SCHOOL DAY ROUTINE BEGINS        " << RESET << std::endl;
	std::cout << YELLOW << "========================================" << RESET << std::endl;
	std::cout << std::endl;
	
	// 1. Launch classes
	std::cout << BLUE << "--- Morning Classes (Session 1) ---" << RESET << std::endl;
	launchClasses();
	std::cout << std::endl;
	
	// 2. Recreation
	std::cout << YELLOW << "--- Morning Break ---" << RESET << std::endl;
	requestRingBell();
	std::cout << std::endl;
	
	// 3. Launch classes
	std::cout << BLUE << "--- Morning Classes (Session 2) ---" << RESET << std::endl;
	launchClasses();
	std::cout << std::endl;
	
	// 4. Lunch
	std::cout << YELLOW << "--- Lunch Time ---" << RESET << std::endl;
	std::cout << MAGENTA << "🍽️  Everyone goes to lunch!" << RESET << std::endl;
	requestRingBell();
	std::cout << std::endl;
	
	// 5. Launch classes
	std::cout << BLUE << "--- Afternoon Classes (Session 1) ---" << RESET << std::endl;
	launchClasses();
	std::cout << std::endl;
	
	// 6. Recreation
	std::cout << YELLOW << "--- Afternoon Break ---" << RESET << std::endl;
	requestRingBell();
	std::cout << std::endl;
	
	// 7. Launch classes
	std::cout << BLUE << "--- Afternoon Classes (Session 2) ---" << RESET << std::endl;
	launchClasses();
	std::cout << std::endl;
	
	std::cout << YELLOW << "========================================" << RESET << std::endl;
	std::cout << YELLOW << "        SCHOOL DAY ROUTINE ENDS         " << RESET << std::endl;
	std::cout << YELLOW << "========================================" << RESET << std::endl;
	std::cout << std::endl;
}

Course* School::getCourse(std::string name)
{
	std::vector<Course*>& courses = CourseList::getInstance().getAll();
	for (std::vector<Course*>::iterator it = courses.begin(); it != courses.end(); ++it)
	{
		if ((*it)->getName() == name)
			return *it;
	}
	return NULL;
}

std::vector<Student*> School::getStudents()
{
	return _students;
}

std::vector<Professor*> School::getProfessors()
{
	return _professors;
}

Headmaster* School::getHeadmaster()
{
	return _headmaster;
}

Secretary* School::getSecretary()
{
	return _secretary;
}

void School::graduationCeremony()
{
	std::cout << std::endl;
	std::cout << CYAN << "========================================" << RESET << std::endl;
	std::cout << CYAN << "       GRADUATION CEREMONY BEGINS       " << RESET << std::endl;
	std::cout << CYAN << "========================================" << RESET << std::endl;
	std::cout << std::endl;
	
	// Notify all observers that ceremony is starting
	_headmaster->getBell()->notify(GraduationCeremony);
	std::cout << std::endl;
	
	bool anyGraduations = false;
	
	for (std::vector<Student*>::iterator studentIt = _students.begin(); studentIt != _students.end(); ++studentIt)
	{
		Student* student = *studentIt;
		
		// Check all courses
		std::vector<Course*>& courses = CourseList::getInstance().getAll();
		for (std::vector<Course*>::iterator courseIt = courses.begin(); courseIt != courses.end(); ++courseIt)
		{
			Course* course = *courseIt;
			
			if (course->canGraduate(student))
			{
				// Find the professor for this course
				Professor* professor = NULL;
				for (std::vector<Professor*>::iterator profIt = _professors.begin(); profIt != _professors.end(); ++profIt)
				{
					if ((*profIt)->getCurrentCourse() == course)
					{
						professor = *profIt;
						break;
					}
				}
				
				if (professor)
				{
					professor->requestStudentGraduation(_headmaster, student, course);
					anyGraduations = true;
				}
			}
		}
	}
	
	if (!anyGraduations)
	{
		std::cout << YELLOW << "No students are ready to graduate yet." << RESET << std::endl;
	}
	
	std::cout << std::endl;
	std::cout << CYAN << "========================================" << RESET << std::endl;
	std::cout << CYAN << "        GRADUATION CEREMONY ENDS        " << RESET << std::endl;
	std::cout << CYAN << "========================================" << RESET << std::endl;
	std::cout << std::endl;
}
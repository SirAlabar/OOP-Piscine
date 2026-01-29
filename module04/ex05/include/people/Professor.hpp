#ifndef PROFESSOR_HPP
#define PROFESSOR_HPP

#include "Staff.hpp"
#include "patterns/observer.hpp"

class Course;
class Headmaster;
class Student;
class Classroom;

class Professor : public Staff, public IObserver
{
private:
	Course* _currentCourse;
	bool _onBreak;
	
	Classroom* findAvailableClassroom();

public:
	Professor(std::string p_name);
	~Professor();
	
	void assignCourse(Course* p_course);
	void doClass(Headmaster* headmaster);
	void closeCourse();
	
	void requestCourseCreation(Headmaster* headmaster, std::string courseName);
	void requestStudentGraduation(Headmaster* headmaster, Student* student, Course* course);
	
	void update(Event event);
	
	bool isOnBreak() const { return _onBreak; }
	Course* getCurrentCourse() const { return _currentCourse; }
};

#endif
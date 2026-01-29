#ifndef CLASSROOM_HPP
#define CLASSROOM_HPP

#include "rooms/Room.hpp"
#include <vector>

class Course;
class Professor;
class Student;

class Classroom : public Room
{
private:
	Course* _currentCourse;
	Professor* _currentProfessor;
	std::vector<Student*> _attendingStudents;
	bool _occupied;

public:
	Classroom();
	
	void assignCourse(Course* p_course);
	
	bool isAvailable() const { return !_occupied; }
	void startClass(Professor* professor, Course* course);
	void endClass();
	
	Course* getCurrentCourse() const { return _currentCourse; }
	Professor* getProfessor() const { return _currentProfessor; }
};

#endif
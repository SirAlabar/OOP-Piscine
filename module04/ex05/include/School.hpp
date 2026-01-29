#ifndef SCHOOL_HPP
#define SCHOOL_HPP

#include <vector>
#include <string>

class Headmaster;
class Secretary;
class Professor;
class Student;
class Course;

class School
{
private:
	Headmaster* _headmaster;
	Secretary* _secretary;
	std::vector<Professor*> _professors;
	std::vector<Student*> _students;

public:
	School();
	~School();
	
	// Core routine
    void setupSchool();
	void runDayRoutine();
	void launchClasses();
	void requestRingBell();
	
	// Recruitment
	void recruteProfessor(std::string name);
	void recruteStudent(std::string name);
	
	// Queries
	Course* getCourse(std::string name);
	std::vector<Student*> getStudents();
	std::vector<Professor*> getProfessors();
	Headmaster* getHeadmaster();
	Secretary* getSecretary();
	
	// Graduation
	void graduationCeremony();
};

#endif

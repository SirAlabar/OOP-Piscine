#ifndef COURSE_HPP
#define COURSE_HPP

#include <string>
#include <vector>

class Student;
class Professor;

class Course
{
private:
	std::string _name;
	Professor* _responsable;
	std::vector<Student*> _students;
	int _numberOfClassToGraduate;
	int _maximumNumberOfStudent;

public:
	Course(std::string p_name);
	void assign(Professor* p_professor);
	void subscribe(Student* p_student);
	const std::string& getName() const { return _name; }
};

#endif
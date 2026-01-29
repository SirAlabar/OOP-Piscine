#ifndef COURSE_HPP
#define COURSE_HPP

#include <string>
#include <vector>
#include <map>

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
	std::map<Student*, int> _attendanceCount;

public:
	Course(std::string p_name);
	
	void assign(Professor* p_professor);
	void subscribe(Student* p_student);
	void unsubscribe(Student* p_student);
	
	// Attendance tracking
	void markAttendance(Student* p_student);
	bool canGraduate(Student* p_student) const;
	int getAttendance(Student* p_student) const;
	
	const std::string& getName() const { return _name; }
	Professor* getProfessor() const { return _responsable; }
	const std::vector<Student*>& getStudents() const { return _students; }
	
	void setRequiredClasses(int count) { _numberOfClassToGraduate = count; }
	int getRequiredClasses() const { return _numberOfClassToGraduate; }
};

#endif
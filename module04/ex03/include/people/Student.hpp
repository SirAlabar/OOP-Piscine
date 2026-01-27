#ifndef STUDENT_HPP
#define STUDENT_HPP

#include "Person.hpp"
#include <vector>

class Course;
class Classroom;
class Headmaster;

class Student : public Person
{
private:
	std::vector<Course*> _subscribedCourse;

public:
	Student(std::string p_name);
	~Student();
	
	void attendClass(Classroom* p_classroom);
	void exitClass();
	void graduate(Course* p_course);
	
	void requestCourseSubscription(Headmaster* headmaster, Course* course);
	
	bool isSubscribedTo(Course* course) const;
	size_t getSubscribedCoursesCount() const { return _subscribedCourse.size(); }
};

#endif
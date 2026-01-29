#ifndef STUDENT_HPP
#define STUDENT_HPP

#include "Person.hpp"
#include "patterns/observer.hpp"
#include <vector>

class Course;
class Classroom;
class Headmaster;

class Student : public Person, public IObserver
{
private:
	std::vector<Course*> _subscribedCourse;
	bool _onBreak;

public:
	Student(std::string p_name);
	~Student();
	
	void attendClass(Classroom* p_classroom);
	void exitClass();
	void graduate(Course* p_course);
	
	void requestCourseSubscription(Headmaster* headmaster, Course* course);
	
	void update(Event event);
	
	bool isSubscribedTo(Course* course) const;
	bool isOnBreak() const { return _onBreak; }
	size_t getSubscribedCoursesCount() const { return _subscribedCourse.size(); }
};

#endif
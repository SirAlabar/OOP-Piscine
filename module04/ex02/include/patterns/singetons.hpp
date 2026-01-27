#ifndef SINGETONS_HPP
#define SINGETONS_HPP

#include "patterns/Singleton.hpp"
#include "people/Student.hpp"
#include "people/Staff.hpp"
#include "academic/Course.hpp"
#include "rooms/Room.hpp"

typedef Singleton<Student> StudentList;
typedef Singleton<Staff> StaffList;
typedef Singleton<Course> CourseList;
typedef Singleton<Room> RoomList;

#endif
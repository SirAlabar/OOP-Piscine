#include "utils/Time.hpp"
#include <sstream>
#include <iomanip>

// Default constructor (00h00)
Time::Time() : _hours(0), _minutes(0)
{
}

// Parameterized constructor
Time::Time(int hours, int minutes) : _hours(hours), _minutes(minutes)
{
}

// Parse "HHhMM" format (e.g., "14h10")
Time::Time(const std::string& timeStr) : _hours(0), _minutes(0)
{
	std::istringstream iss(timeStr);
	char delimiter;
	
	if (iss >> _hours >> delimiter >> _minutes && delimiter == 'h')
	{
		// Successfully parsed "14h10" format
	}
	else
	{
		// Parse failed, keep defaults (0, 0)
		_hours = 0;
		_minutes = 0;
	}
}

// Copy constructor
Time::Time(const Time& other) : _hours(other._hours), _minutes(other._minutes)
{
}

// Assignment operator
Time& Time::operator=(const Time& other)
{
	if (this != &other)
	{
		_hours = other._hours;
		_minutes = other._minutes;
	}
	return *this;
}

// Destructor
Time::~Time()
{
}

// Getters
int Time::getHours() const
{
	return _hours;
}

int Time::getMinutes() const
{
	return _minutes;
}

// Convert to total minutes (for comparison/arithmetic)
int Time::toMinutes() const
{
	return _hours * 60 + _minutes;
}

// Format as "HHhMM"
std::string Time::toString() const
{
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(2) << _hours << "h";
	oss << std::setfill('0') << std::setw(2) << _minutes;
	return oss.str();
}

// Validation
bool Time::isValid() const
{
	return _hours >= 0 && _hours < 24 && _minutes >= 0 && _minutes < 60;
}

// Comparison operators
bool Time::operator==(const Time& other) const
{
	return toMinutes() == other.toMinutes();
}

bool Time::operator!=(const Time& other) const
{
	return !(*this == other);
}

bool Time::operator<(const Time& other) const
{
	return toMinutes() < other.toMinutes();
}

bool Time::operator<=(const Time& other) const
{
	return toMinutes() <= other.toMinutes();
}

bool Time::operator>(const Time& other) const
{
	return toMinutes() > other.toMinutes();
}

bool Time::operator>=(const Time& other) const
{
	return toMinutes() >= other.toMinutes();
}

// Add durations
Time Time::operator+(const Time& other) const
{
	int totalMinutes = toMinutes() + other.toMinutes();
	return Time(totalMinutes / 60, totalMinutes % 60);
}

// Subtract durations
Time Time::operator-(const Time& other) const
{
	int totalMinutes = toMinutes() - other.toMinutes();
	if (totalMinutes < 0)
	{
		totalMinutes = 0;
	}
	return Time(totalMinutes / 60, totalMinutes % 60);
}
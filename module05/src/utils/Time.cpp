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

    if (!(iss >> _hours >> delimiter >> _minutes) || delimiter != 'h')
    {
        _hours   = 0;
        _minutes = 0;
    }
}

// Static factory: construct from a raw seconds value (simulation clock)
Time Time::fromSeconds(double seconds)
{
    int totalMinutes = static_cast<int>(seconds) / 60;
    return Time(totalMinutes / 60, totalMinutes % 60);
}

// Getters
int Time::getHours()   const
{
	return _hours;
}

int Time::getMinutes() const
{
	return _minutes;
}

int Time::toMinutes() const
{
    return _hours * 60 + _minutes;
}

// Format as "HHhMM"
std::string Time::toString() const
{
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << _hours   << "h"
        << std::setfill('0') << std::setw(2) << _minutes;
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

bool Time::operator<(const Time& other)  const
{
	return toMinutes() <  other.toMinutes();
}

bool Time::operator<=(const Time& other) const
{
	return toMinutes() <= other.toMinutes();
}

bool Time::operator>(const Time& other)  const
{
	return toMinutes() >  other.toMinutes();
}

bool Time::operator>=(const Time& other) const
{
	return toMinutes() >= other.toMinutes();
}

// Add durations
Time Time::operator+(const Time& other) const
{
    int total = toMinutes() + other.toMinutes();
    return Time(total / 60, total % 60);
}

// Subtract durations (clamp to zero)
Time Time::operator-(const Time& other) const
{
    int total = toMinutes() - other.toMinutes();
    if (total < 0)
	{
		total = 0;
	}
    return Time(total / 60, total % 60);
}
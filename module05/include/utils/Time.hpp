#ifndef TIME_HPP
#define TIME_HPP

#include <string>

// Represents time in HHhMM format (e.g., "14h10" = 14 hours 10 minutes)
class Time
{
private:
	int _hours;
	int _minutes;

public:
	Time();
	Time(int hours, int minutes);
	Time(const std::string& timeStr);  // Parse "HHhMM" format
	Time(const Time&) = default;
	Time& operator=(const Time&) = default;
	~Time() = default;

	int getHours() const;
	int getMinutes() const;
	int toMinutes() const;  // Convert to total minutes for comparison
	std::string toString() const;  // Format as "HHhMM"
	bool isValid() const;  // Hours: 0-23, Minutes: 0-59

	// Comparison operators
	bool operator==(const Time& other) const;
	bool operator!=(const Time& other) const;
	bool operator<(const Time& other) const;
	bool operator<=(const Time& other) const;
	bool operator>(const Time& other) const;
	bool operator>=(const Time& other) const;

	// Arithmetic
	Time operator+(const Time& other) const;  // Add durations
	Time operator-(const Time& other) const;  // Subtract durations
};

#endif

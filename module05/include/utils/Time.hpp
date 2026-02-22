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
    Time(const Time&)            = default;
    Time& operator=(const Time&) = default;
    ~Time()                      = default;

    int         getHours()   const;
    int         getMinutes() const;
    int         toMinutes()  const;
    double      toSeconds()  const;
    std::string toString()   const;   // Format as "HHhMM"
    bool        isValid()    const;   // Hours: 0-23, Minutes: 0-59

    // Construct a Time from a raw seconds value (e.g. simulation clock).
    // Avoids repeating the seconds→hours/minutes conversion everywhere.
    static Time fromSeconds(double seconds);

    // Comparison operators
    bool operator==(const Time& other) const;
    bool operator!=(const Time& other) const;
    bool operator<(const Time& other)  const;
    bool operator<=(const Time& other) const;
    bool operator>(const Time& other)  const;
    bool operator>=(const Time& other) const;

    // Arithmetic
    Time operator+(const Time& other) const;
    Time operator-(const Time& other) const;
};

#endif
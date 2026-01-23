#ifndef HOURLYEMPLOYEE_HPP
#define HOURLYEMPLOYEE_HPP

#include "employee.hpp"

class TempWorker : public Employee
{
private:
    int hoursWorked;

public:
    TempWorker(int hourlyValue)
        : Employee(hourlyValue), hoursWorked(0)
    {
    }

    int executeWorkday()
    {
        hoursWorked += 7;
        return 7;
    }

    void mobilize(int hours)
    {
        hoursWorked += hours;
    }

    double calculateMonthlyPayroll() const
    {
        return hoursWorked * hourlyValue;
    }

    int getHoursWorked() const
    {
        return hoursWorked;
    }
};

class Apprentice : public Employee
{
private:
    int workHours;
    int schoolHours;

public:
    Apprentice(int hourlyValue)
        : Employee(hourlyValue), workHours(0), schoolHours(0)
    {
    }

    int executeWorkday()
    {
        workHours += 7;
        return 7;
    }

    void registerSchoolHours(int hours)
    {
        schoolHours += hours;
    }

    double calculateMonthlyPayroll() const
    {
        return workHours * hourlyValue + schoolHours * (hourlyValue / 2.0);
    }

    int getWorkHours() const
    {
        return workHours;
    }

    int getSchoolHours() const
    {
        return schoolHours;
    }
};

#endif
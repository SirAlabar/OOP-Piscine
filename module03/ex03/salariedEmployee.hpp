#ifndef SALARIEDEMPLOYEE_HPP
#define SALARIEDEMPLOYEE_HPP

#include "employee.hpp"

class ContractEmployee : public Employee
{
private:
    int expectedMonthlyHours;
    int absenceHours;

public:
    ContractEmployee(int hourlyValue, int expectedMonthlyHours)
        : Employee(hourlyValue), expectedMonthlyHours(expectedMonthlyHours), absenceHours(0)
    {
    }

    int executeWorkday()
    {
        return 7;
    }

    void registerAbsence(int hours)
    {
        absenceHours += hours;
    }

    double calculateMonthlyPayroll() const
    {
        int effectiveHours = expectedMonthlyHours - absenceHours;
        if (effectiveHours < 0)
        {
            effectiveHours = 0;
        }
        return effectiveHours * hourlyValue;
    }

    int getAbsenceHours() const
    {
        return absenceHours;
    }

    int getExpectedHours() const
    {
        return expectedMonthlyHours;
    }
};

#endif
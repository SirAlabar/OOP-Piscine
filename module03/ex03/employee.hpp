#ifndef EMPLOYEE_HPP
#define EMPLOYEE_HPP

class Employee
{
protected:
    int hourlyValue;

public:
    Employee(int hourlyValue)
        : hourlyValue(hourlyValue)
    {
    }

    virtual ~Employee()
    {
    }

    virtual int executeWorkday() = 0;
    virtual double calculateMonthlyPayroll() const = 0;
};

#endif
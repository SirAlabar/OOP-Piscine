#ifndef EMPLOYEEMANAGER_HPP
#define EMPLOYEEMANAGER_HPP

#include <vector>
#include <iostream>
#include <iomanip>
#include "employee.hpp"

class EmployeeManager
{
private:
    std::vector<Employee*> employees;

public:
    void addEmployee(Employee* employee)
    {
        employees.push_back(employee);
    }

    void removeEmployee(Employee* employee)
    {
        for (size_t i = 0; i < employees.size(); ++i)
        {
            if (employees[i] == employee)
            {
                employees.erase(employees.begin() + i);
                return;
            }
        }
    }

    void executeWorkday()
    {
        for (size_t i = 0; i < employees.size(); ++i)
        {
            employees[i]->executeWorkday();
        }
    }

    void calculatePayroll() const
    {
        std::cout << "\n\033[1m\033[36m=== MONTHLY PAYROLL ===\033[0m\n" << std::endl;
        
        double total = 0.0;
        for (size_t i = 0; i < employees.size(); ++i)
        {
            double payment = employees[i]->calculateMonthlyPayroll();
            total += payment;
            
            std::cout << "\033[32mEmployee #" << (i + 1) << ": " 
                      << std::fixed << std::setprecision(2) 
                      << payment << " EUR\033[0m" << std::endl;
        }
        
        std::cout << "\n\033[1m\033[33mTotal Payroll: " 
                  << std::fixed << std::setprecision(2) 
                  << total << " EUR\033[0m\n" << std::endl;
    }
};

#endif
#include <iostream>
#include "employeeManager.hpp"
#include "hourlyEmployee.hpp"
#include "salariedEmployee.hpp"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

int main()
{
    std::cout << BOLD << CYAN << "  INTERFACE SEGREGATION PRINCIPLE DEMO " << RESET << std::endl;
    std::cout << std::endl;

    EmployeeManager manager;

    std::cout << YELLOW << BOLD << ">>> Creating Employees <<<" << RESET << std::endl;
    std::cout << std::endl;

    TempWorker tempWorker(20);
    std::cout << GREEN << "✓ TempWorker created (hourly rate: 20 EUR)" << RESET << std::endl;

    ContractEmployee contract(25, 140);
    std::cout << BLUE << "✓ ContractEmployee created (hourly rate: 25 EUR, expected: 140h/month)" << RESET << std::endl;

    Apprentice apprentice(15);
    std::cout << MAGENTA << "✓ Apprentice created (hourly rate: 15 EUR, school: 7.5 EUR)" << RESET << std::endl;
    std::cout << std::endl;

    std::cout << YELLOW << BOLD << ">>> Adding Employees to Manager <<<" << RESET << std::endl;
    manager.addEmployee(&tempWorker);
    manager.addEmployee(&contract);
    manager.addEmployee(&apprentice);
    std::cout << GREEN << "✓ All employees added" << RESET << std::endl;
    std::cout << std::endl;

    std::cout << YELLOW << BOLD << ">>> Simulating Work Month <<<" << RESET << std::endl;
    std::cout << std::endl;

    std::cout << CYAN << "Week 1: 5 workdays" << RESET << std::endl;
    for (int i = 0; i < 5; ++i)
    {
        manager.executeWorkday();
    }
    std::cout << GREEN << "✓ Week 1 completed (35 hours each)" << RESET << std::endl;
    std::cout << std::endl;

    std::cout << CYAN << "Week 2: 5 workdays + apprentice goes to school" << RESET << std::endl;
    for (int i = 0; i < 5; ++i)
    {
        manager.executeWorkday();
    }
    apprentice.registerSchoolHours(14);
    std::cout << GREEN << "✓ Week 2 completed (35 work hours + 14 school hours for apprentice)" << RESET << std::endl;
    std::cout << std::endl;

    std::cout << CYAN << "Week 3: 5 workdays + contract employee sick 2 days" << RESET << std::endl;
    for (int i = 0; i < 3; ++i)
    {
        manager.executeWorkday();
    }
    contract.registerAbsence(14);
    std::cout << RED << "✗ ContractEmployee absent 2 days (14 hours)" << RESET << std::endl;
    std::cout << GREEN << "✓ Week 3 completed" << RESET << std::endl;
    std::cout << std::endl;

    std::cout << CYAN << "Week 4: 5 workdays + temp worker mobilized for extra 10 hours" << RESET << std::endl;
    for (int i = 0; i < 5; ++i)
    {
        manager.executeWorkday();
    }
    tempWorker.mobilize(10);
    std::cout << YELLOW << "⚡ TempWorker mobilized for 10 extra hours" << RESET << std::endl;
    std::cout << GREEN << "✓ Week 4 completed" << RESET << std::endl;
    std::cout << std::endl;

    std::cout << YELLOW << BOLD << ">>> Employee Statistics <<<" << RESET << std::endl;
    std::cout << std::endl;

    std::cout << GREEN << "TempWorker:" << std::endl;
    std::cout << "  Hours worked: " << tempWorker.getHoursWorked() << " hours" << RESET << std::endl;
    std::cout << std::endl;

    std::cout << BLUE << "ContractEmployee:" << std::endl;
    std::cout << "  Expected hours: " << contract.getExpectedHours() << " hours" << std::endl;
    std::cout << "  Absence hours: " << contract.getAbsenceHours() << " hours" << std::endl;
    std::cout << "  Effective hours: " << (contract.getExpectedHours() - contract.getAbsenceHours()) << " hours" << RESET << std::endl;
    std::cout << std::endl;

    std::cout << MAGENTA << "Apprentice:" << std::endl;
    std::cout << "  Work hours: " << apprentice.getWorkHours() << " hours" << std::endl;
    std::cout << "  School hours: " << apprentice.getSchoolHours() << " hours (paid at half rate)" << RESET << std::endl;
    std::cout << std::endl;

    manager.calculatePayroll();

    std::cout << CYAN << BOLD << "✓ Interface Segregation Principle satisfied!" << RESET << std::endl;
    std::cout << CYAN << "  Each employee type has only the methods it needs" << RESET << std::endl;
    std::cout << CYAN << "  - TempWorker: mobilize()" << std::endl;
    std::cout << CYAN << "  - ContractEmployee: registerAbsence()" << std::endl;
    std::cout << CYAN << "  - Apprentice: registerSchoolHours()" << RESET << std::endl;

    return 0;
}
#ifndef BANK_HPP
#define BANK_HPP

#include <map>
#include <iostream>
#include "Account.hpp"

class Bank
{
private:
    double _liquidity;
    std::map<int, Account> _clientAccounts;

public:
    //Canonical form
    Bank();
    Bank(const Bank& other);
    Bank& operator=(const Bank& other);
    ~Bank();

    void createAccount(int id, double value, const std::string& owner);
    void deleteAccount(int id);
    void modifyAccount(int id, double newValue);
    void giveLoan(double amount);
    void deposit(int id, double amount);
    void withdraw(int id, double amount);

    friend std::ostream& operator<<(std::ostream& os, const Bank& bank);
};

#endif
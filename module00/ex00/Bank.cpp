#include "Bank.hpp"
#include <iomanip>

//constructor
Bank::Bank() : _liquidity(0.0)
{
}

//copy constructor
Bank::Bank(const Bank& other) : 
    _liquidity(other._liquidity),
    _clientAccounts(other._clientAccounts)
{
}

//Copy assigment operator
Bank& Bank::operator=(const Bank& other)
{
    if (this != &other)
    {
        _liquidity = other._liquidity;
        _clientAccounts = other._clientAccounts;
    }
    return (*this);
}

//Destructor
Bank::~Bank()
{
}

void Bank::createAccount(int id, double value, const std::string& owner)
{
    if (_clientAccounts.find(id) != _clientAccounts.end())
    {
        std::cout << "Error: Account ID " << id << " already exists" << std::endl;
        return;
    }

    Account newAccount(id, value, owner);

    _clientAccounts[id] = newAccount;
    std::cout << "Account created: " << newAccount << std::endl;
}

void Bank::deleteAccount(int id)
{
    if (_clientAccounts.find(id) == _clientAccounts.end())
    {
        std::cout << "Error: Account ID " << id << " not found" << std::endl;
        return;
    }
    
    _clientAccounts.erase(id);
    std::cout << "Account " << id << " deleted" << std::endl;
}

void Bank::modifyAccount(int id, double newValue)
{
    if (_clientAccounts.find(id) == _clientAccounts.end())
    {
        std::cout << "Error: Account ID " << id << " not found" << std::endl;
        return;
    }

    Account acc = _clientAccounts[id];
    acc.setValue(newValue);
    _clientAccounts[id] = acc;

    std::cout << "Account " << id << " modified to " << newValue << std::endl;
}

void Bank::giveLoan(double amount)
{
    _liquidity += amount;
    std::cout << "Loan of " << amount << " received. Bank liquidity: " << _liquidity << std::endl;    
}

void Bank::deposit(int id, double amount)
{
    if (_clientAccounts.find(id) == _clientAccounts.end())
    {
        std::cout << "Error: Account ID " << id << " not found" << std::endl;
        return;
    }

    double bankFee = amount * 0.05;
    double clientAmount = amount - bankFee;

    Account acc = _clientAccounts[id];
    acc.setValue(acc.getValue() + clientAmount);
    _clientAccounts[id] = acc;
    _liquidity += bankFee;

    std::cout << "Deposit " << amount << " to account " << id 
            << " (fee: " << bankFee << ", client: " << clientAmount << ")" << std::endl;
}

void Bank::withdraw(int id, double amount)
{
    if (_clientAccounts.find(id) == _clientAccounts.end())
    {
        std::cout << "Error: Account ID " << id << " not found" << std::endl;
        return;
    }

    if (_clientAccounts[id].getValue() < amount)
    {
        std::cout << "Error: Insufficient funds in account " << id << std::endl;
        return;
    }

    Account acc = _clientAccounts[id];
    acc.setValue(acc.getValue() - amount);
    _clientAccounts[id] = acc;
    
    std::cout << "Withdrawal " << amount << " from account " << id << std::endl;  
}

std::ostream& operator<<(std::ostream& os, const Bank& bank)
{
    os << "Bank informations:" << std::endl;
    os << "Liquidity: " << std::fixed << std::setprecision(2) << bank._liquidity << std::endl;
    for (std::map<int, Account>::const_iterator it = bank._clientAccounts.begin();
        it != bank._clientAccounts.end(); ++it)
    {
        os << it->second << std::endl;
    }
    return os;
}
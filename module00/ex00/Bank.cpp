#include "Bank.hpp"
#include <iomanip>

//constructor
Bank::Account::Account() : _id(-1), _value(0.0), _owner("")
{
}

Bank::Account::Account(int id, double value, const std::string& owner) :
    _id(id), _value(value), _owner(owner)
{
}

//copy constructor
Bank::Account::Account(const Account& other) : 
    _id(other._id),
    _value(other._value),
    _owner(other._owner)
{  
}

//Copy assigment operator
Bank::Account& Bank::Account::operator=(const Account& other)
{
    if (this != &other)
    {
        _id = other._id;
        _value = other._value;
        _owner = other._owner;
    }
    return (*this);
}

//Destructor
Bank::Account::~Account()
{
}

//Gets
int Bank::Account::getId() const
{
    return (_id);
}

double Bank::Account::getValue() const
{
    return(_value);
}

const std::string& Bank::Account::getOwner() const
{
    return(_owner);
}

std::ostream& operator<<(std::ostream& os, const Bank::Account& account)
{
    os << "[" << account._id << "] - [" << std::fixed << std::setprecision(2)
       << account._value << "] - [" << account._owner << "]";
    return (os);
}

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
        throw std::runtime_error("Account ID already exists");
    }

    Account newAccount(id, value, owner);
    _clientAccounts.insert(std::map<int, Account>::value_type(id, newAccount));
    std::cout << "Account created: " << newAccount << std::endl;
}

void Bank::deleteAccount(int id)
{
    if (_clientAccounts.find(id) == _clientAccounts.end())
    {
        throw std::runtime_error("Account ID not found");
    }
    
    _clientAccounts.erase(id);
    std::cout << "Account " << id << " deleted" << std::endl;
}

void Bank::modifyAccount(int id, double newValue)
{
    if (_clientAccounts.find(id) == _clientAccounts.end())
    {
        throw std::runtime_error("Account ID not found");
    }

    std::map<int, Account>::iterator it = _clientAccounts.find(id);
    it->second._value = newValue;
    std::cout << std::fixed << std::setprecision(2)
              << "Account " << id << " modified to " << newValue << std::endl;
}

void Bank::giveLoan(int id, double amount)
{
    std::map<int, Account>::iterator it = _clientAccounts.find(id);
    
    if (it == _clientAccounts.end())
    {
        throw std::runtime_error("Account ID not found");
    }
    
    if (_liquidity < amount)
    {
        throw std::runtime_error("Insufficient bank liquidity for loan");
    }
    
    _liquidity -= amount;
    it->second._value += amount;
    
    std::cout << std::fixed << std::setprecision(2)
              << "Loan of " << amount << " given to account " << id 
              << ". Bank liquidity: " << _liquidity << std::endl;
}

void Bank::deposit(int id, double amount)
{
    if (_clientAccounts.find(id) == _clientAccounts.end())
    {
        throw std::runtime_error("Account ID not found");
    }

    double bankFee = amount * 0.05;
    double clientAmount = amount - bankFee;

    std::map<int, Account>::iterator it = _clientAccounts.find(id);
    it->second._value += clientAmount;
    _liquidity += bankFee;

    std::cout << std::fixed << std::setprecision(2)
                << "Deposit " << amount << " to account " << id 
                << " (fee: " << bankFee << ", client: " << clientAmount << ")" << std::endl;
}

void Bank::withdraw(int id, double amount)
{
    std::map<int, Account>::iterator it = _clientAccounts.find(id);
    
    if (it == _clientAccounts.end())
    {
        throw std::runtime_error("Account ID not found");
    }
    
    if (it->second.getValue() < amount)
    {
        throw std::runtime_error("Insufficient funds");
    }
    
    it->second._value -= amount;    
    std::cout << std::fixed << std::setprecision(2)
              << "Withdrawal " << amount << " from account " << id << std::endl;  
}

Bank::Account& Bank::operator[](int id)
{
    std::map<int, Account>::iterator it = _clientAccounts.find(id);
    if (it == _clientAccounts.end())
    {
        throw std::runtime_error("Account ID not found");
    }
    return (it->second);
}

const Bank::Account& Bank::operator[](int id) const
{
    std::map<int, Account>::const_iterator it = _clientAccounts.find(id);
    if (it == _clientAccounts.end())
    {
        throw std::runtime_error("Account ID not found");
    }
    return (it->second);
}

std::ostream& operator<<(std::ostream& os, const Bank& bank)
{
    os << "Bank informations:" << std::endl;
    os << "Liquidity: " << std::fixed << std::setprecision(2) << bank._liquidity << std::endl;
    for (std::map<int, Bank::Account>::const_iterator it = bank._clientAccounts.begin();
        it != bank._clientAccounts.end(); ++it)
    {
        os << it->second << std::endl;
    }
    return os;
}
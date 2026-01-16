#include "Account.hpp"
#include <iomanip>
#include <sstream>

//constructor
Account::Account() : _id(-1), _value(0.0), _owner("")
{
}

Account::Account(int id, double value, const std::string& owner) :
    _id(id), _value(value), _owner(owner)
{
}

//copy constructor
Account::Account(const Account& other) : 
    _id(other._id),
    _value(other._value),
    _owner(other._owner)
{  
}

//Copy assigment operator
Account& Account::operator=(const Account& other)
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
Account::~Account()
{
}

//Gets
int Account::getId() const
{
    return (_id);
}

double Account::getValue() const
{
    return(_value);
}

const std::string& Account::getOwner() const
{
    return(_owner);
}

//Sets
void Account::setValue(double value) 
{ 
    _value = value; 
}

std::ostream& operator<<(std::ostream& os, const Account& account)
{
    os << "[" << account._id << "] - [" << std::fixed << std::setprecision(2) 
       << account._value << "] - [" << account._owner << "]";
    return os;
}
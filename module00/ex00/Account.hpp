#ifndef ACCOUNT_HPP
#define ACCOUNT_HPP

#include <string>
#include <iostream>

class Account
{
private:
    int _id;
    double _value;
    std::string _owner;

public:
    //Canonical form
    Account();
    Account(int id, double value, const std::string& owner);
    Account(const Account& other);
    Account& operator=(const Account& other);
    ~Account();

    //Gets
    int getId() const;
    double getValue() const;
    const std::string& getOwner() const;

    //Sets
    void setValue(double value);

    friend std::ostream& operator<<(std::ostream& os, const Account& account);
};

#endif
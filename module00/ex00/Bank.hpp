#ifndef BANK_HPP
#define BANK_HPP

#include <map>
#include <iostream>
#include <string>
#include <stdexcept>

class Bank
{   
private:
    // Internal Account class
    class Account
    {
    private:
        int _id;
        double _value;
        std::string _owner;
        
        Account();
        Account(int id, double value, const std::string& owner);
        
        friend class Bank;
        
    public:
        // Copy constructor
        Account(const Account& other);
        // Copy assignment
        Account& operator=(const Account& other);
        // Destructor
        ~Account();
        
        //getters
        int getId() const;
        double getValue() const;
        const std::string& getOwner() const;

        friend std::ostream& operator<<(std::ostream& os, const Account& account);
    };
    
    double _liquidity;
    std::map<int, Account> _clientAccounts;

public:
    //Canonical form
    Bank();
    Bank(const Bank& other);
    Bank& operator=(const Bank& other);
    ~Bank();

    //Methods
    void createAccount(int id, double value, const std::string& owner);
    void deleteAccount(int id);
    void modifyAccount(int id, double newValue);
    void giveLoan(int id, double amount);
    void deposit(int id, double amount);
    void withdraw(int id, double amount);

    //Operator[]
    Account& operator[](int id);
    const Account& operator[](int id) const;
    
    //Output
    friend std::ostream& operator<<(std::ostream& os, const Account& account);
    friend std::ostream& operator<<(std::ostream& os, const Bank& bank);
};

#endif
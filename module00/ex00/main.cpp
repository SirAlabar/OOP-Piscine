#include "Bank.hpp"
#include <iostream>

int main()
{
    Bank bank;
    
    std::cout << "=== INITIAL STATE ===" << std::endl;
    std::cout << bank << std::endl;
    
    std::cout << "=== BANK RECEIVES LOAN ===" << std::endl;
    bank.giveLoan(15750.50);
    std::cout << std::endl;
    
    std::cout << "=== CREATE ACCOUNTS ===" << std::endl;
    bank.createAccount(1, 1234.56, "Alice");
    bank.createAccount(2, 789.45, "Bob");
    bank.createAccount(3, 456.78, "Charlie");
    std::cout << "\nTry duplicate ID:" << std::endl;
    bank.createAccount(1, 100.25, "David");
    std::cout << std::endl;
    
    std::cout << "=== BANK STATUS ===" << std::endl;
    std::cout << bank << std::endl;
    
    std::cout << "=== ALICE DEPOSITS 327.80 (5% bank fee) ===" << std::endl;
    bank.deposit(1, 327.80);
    std::cout << std::endl;
    
    std::cout << "=== BOB WITHDRAWS 123.45 ===" << std::endl;
    bank.withdraw(2, 123.45);
    std::cout << std::endl;
    
    std::cout << "=== CHARLIE TRIES TO WITHDRAW 999.99 (insufficient) ===" << std::endl;
    bank.withdraw(3, 999.99);
    std::cout << std::endl;
    
    std::cout << "=== TRY WITHDRAW FROM NON-EXISTENT ACCOUNT ===" << std::endl;
    bank.withdraw(99, 50.75);
    std::cout << std::endl;
    
    std::cout << "=== MODIFY BOB'S ACCOUNT TO 2500.90 ===" << std::endl;
    bank.modifyAccount(2, 2500.90);
    std::cout << std::endl;
    
    std::cout << "=== TRY MODIFY NON-EXISTENT ACCOUNT ===" << std::endl;
    bank.modifyAccount(99, 100.10);
    std::cout << std::endl;
    
    std::cout << "=== CURRENT BANK STATUS ===" << std::endl;
    std::cout << bank << std::endl;
    
    std::cout << "=== DELETE CHARLIE'S ACCOUNT ===" << std::endl;
    bank.deleteAccount(3);
    std::cout << std::endl;
    
    std::cout << "=== TRY DELETE NON-EXISTENT ACCOUNT ===" << std::endl;
    bank.deleteAccount(99);
    std::cout << std::endl;
    
    std::cout << "=== FINAL BANK STATUS ===" << std::endl;
    std::cout << bank << std::endl;
    
    return 0;
}
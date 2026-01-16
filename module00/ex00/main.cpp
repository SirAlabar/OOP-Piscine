#include "Bank.hpp"
#include <iostream>

int main()
{
    Bank bank;
    
    try
    {
        std::cout << "=== INITIAL STATE ===" << std::endl;
        std::cout << bank << std::endl;
        
        std::cout << "=== CREATE ACCOUNTS ===" << std::endl;
        bank.createAccount(1, 1234.56, "Alice");
        bank.createAccount(2, 789.45, "Bob");
        bank.createAccount(3, 456.78, "Charlie");
        std::cout << std::endl;
        
        std::cout << "=== ALICE DEPOSITS 5000.00 (5% bank fee) ===" << std::endl;
        bank.deposit(1, 5000.00);
        std::cout << std::endl;
        
        std::cout << "=== BOB DEPOSITS 10000.00 (5% bank fee) ===" << std::endl;
        bank.deposit(2, 10000.00);
        std::cout << std::endl;
        
        std::cout << "=== BANK STATUS AFTER DEPOSITS ===" << std::endl;
        std::cout << bank << std::endl;
        
        std::cout << "=== BANK GIVES LOAN OF 500.00 TO CHARLIE ===" << std::endl;
        bank.giveLoan(3, 500.00);
        std::cout << std::endl;
        
        std::cout << "=== TRY DUPLICATE ID ===" << std::endl;
        try 
        {
            bank.createAccount(1, 100.25, "David");
        } 
        catch (const std::runtime_error& e) 
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        std::cout << "=== BANK STATUS ===" << std::endl;
        std::cout << bank << std::endl;
        
        std::cout << "=== BONUS: OPERATOR[] TESTS ===" << std::endl;
        std::cout << "Access Alice using operator[]: " << bank[1] << std::endl;
        std::cout << "Access Bob using operator[]: " << bank[2] << std::endl;
        std::cout << "Access Charlie using operator[]: " << bank[3] << std::endl;
        std::cout << std::endl;
        
        std::cout << "=== OPERATOR[] - ACCESS NON-EXISTENT ACCOUNT ===" << std::endl;
        try 
        {
            std::cout << bank[99] << std::endl;
        } 
        catch (const std::runtime_error& e) 
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        std::cout << "=== OPERATOR[] - GET ACCOUNT DETAILS ===" << std::endl;
        std::cout << "Alice's ID: " << bank[1].getId() << std::endl;
        std::cout << "Alice's Balance: " << bank[1].getValue() << std::endl;
        std::cout << "Alice's Owner: " << bank[1].getOwner() << std::endl;
        std::cout << std::endl;
        
        std::cout << "=== BOB WITHDRAWS 123.45 ===" << std::endl;
        bank.withdraw(2, 123.45);
        std::cout << std::endl;
        
        std::cout << "=== CHARLIE TRIES TO WITHDRAW 999.99 (insufficient) ===" << std::endl;
        try 
        {
            bank.withdraw(3, 999.99);
        } 
        catch (const std::runtime_error& e) 
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        std::cout << "=== TRY GIVE LOAN EXCEEDING BANK LIQUIDITY ===" << std::endl;
        try 
        {
            bank.giveLoan(1, 999999.99);
        } 
        catch (const std::runtime_error& e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        std::cout << "=== TRY GIVE LOAN TO NON-EXISTENT ACCOUNT ===" << std::endl;
        try 
        {
            bank.giveLoan(99, 100.00);
        } 
        catch (const std::runtime_error& e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        std::cout << "=== TRY WITHDRAW FROM NON-EXISTENT ACCOUNT ===" << std::endl;
        try 
        {
            bank.withdraw(99, 50.75);
        } 
        catch (const std::runtime_error& e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        std::cout << "=== MODIFY BOB'S ACCOUNT TO 2500.90 ===" << std::endl;
        bank.modifyAccount(2, 2500.90);
        std::cout << std::endl;
        
        std::cout << "=== TRY MODIFY NON-EXISTENT ACCOUNT ===" << std::endl;
        try 
        {
            bank.modifyAccount(99, 100.10);
        } 
        catch (const std::runtime_error& e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        std::cout << "=== CURRENT BANK STATUS ===" << std::endl;
        std::cout << bank << std::endl;
        
        std::cout << "=== OPERATOR[] - VERIFY UPDATED VALUES ===" << std::endl;
        std::cout << "Alice after deposits: " << bank[1] << std::endl;
        std::cout << "Bob after modification: " << bank[2] << std::endl;
        std::cout << std::endl;
        
        std::cout << "=== DELETE CHARLIE'S ACCOUNT ===" << std::endl;
        bank.deleteAccount(3);
        std::cout << std::endl;
        
        std::cout << "=== OPERATOR[] - TRY ACCESS DELETED ACCOUNT ===" << std::endl;
        try 
        {
            std::cout << bank[3] << std::endl;
        } 
        catch (const std::runtime_error& e) 
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        std::cout << "=== TRY DELETE NON-EXISTENT ACCOUNT ===" << std::endl;
        try 
        {
            bank.deleteAccount(99);
        } 
        catch (const std::runtime_error& e) 
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        std::cout << "=== FINAL BANK STATUS ===" << std::endl;
        std::cout << bank << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
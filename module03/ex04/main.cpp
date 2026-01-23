#include <iostream>
#include <vector>
#include "ILogger.hpp"
#include "HeaderProvider.hpp"
#include "FileLogger.hpp"
#include "StreamLogger.hpp"

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

    std::cout << BOLD << CYAN << " DEPENDENCY INVERSION PRINCIPLE DEMO ║" << RESET << std::endl;
    std::cout << std::endl;

    std::cout << YELLOW << BOLD << ">>> Creating Header Providers <<<" << RESET << std::endl;
    std::cout << std::endl;

    ConstantHeader appHeader("[APP] ");
    std::cout << GREEN << "✓ ConstantHeader: \"[APP] \"" << RESET << std::endl;

    ConstantHeader consoleHeader("[CONSOLE] ");
    std::cout << GREEN << "✓ ConstantHeader: \"[CONSOLE] \"" << RESET << std::endl;

    DateHeader dateHeader;
    std::cout << GREEN << "✓ DateHeader: timestamp format" << RESET << std::endl;
    std::cout << std::endl;

    std::cout << YELLOW << BOLD << ">>> Creating Logger Instances <<<" << RESET << std::endl;
    std::cout << std::endl;

    FileLogger fileLogger1("application.log");
    std::cout << GREEN << "✓ FileLogger (no header) -> application.log" << RESET << std::endl;

    FileLogger fileLogger2("application_const.log", &appHeader);
    std::cout << GREEN << "✓ FileLogger with constant header -> application_const.log" << RESET << std::endl;

    FileLogger fileLogger3("application_date.log", &dateHeader);
    std::cout << GREEN << "✓ FileLogger with date header -> application_date.log" << RESET << std::endl;

    StreamLogger streamLogger1(&std::cout);
    std::cout << BLUE << "✓ StreamLogger (std::cout, no header)" << RESET << std::endl;

    StreamLogger streamLogger2(&std::cout, &consoleHeader);
    std::cout << BLUE << "✓ StreamLogger with constant header (std::cout)" << RESET << std::endl;

    StreamLogger streamLogger3(&std::cerr, &dateHeader);
    std::cout << MAGENTA << "✓ StreamLogger with date header (std::cerr)" << RESET << std::endl;
    std::cout << std::endl;

    std::cout << YELLOW << BOLD << ">>> Building Logger Collection <<<" << RESET << std::endl;
    std::cout << std::endl;

    //DIP
    std::vector<ILogger*> loggers;
    loggers.push_back(&fileLogger1);
    loggers.push_back(&fileLogger2);
    loggers.push_back(&fileLogger3);
    loggers.push_back(&streamLogger1);
    loggers.push_back(&streamLogger2);
    loggers.push_back(&streamLogger3);

    std::cout << GREEN << "✓ " << loggers.size() << " loggers registered" << RESET << std::endl;
    std::cout << std::endl;

    std::cout << YELLOW << BOLD << ">>> Preparing Log Messages <<<" << RESET << std::endl;
    std::cout << std::endl;

    std::vector<std::string> messages;
    messages.push_back("Application started successfully");
    messages.push_back("User authentication completed");
    messages.push_back("Database connection established");
    messages.push_back("Processing batch job #12345");
    messages.push_back("Warning: High memory usage detected");
    messages.push_back("Application shutdown initiated");

    std::cout << GREEN << "✓ " << messages.size() << " messages prepared" << RESET << std::endl;
    std::cout << std::endl;

    std::cout << YELLOW << BOLD << ">>> Broadcasting Messages to All Loggers <<<" << RESET << std::endl;
    std::cout << std::endl;

    for (size_t i = 0; i < messages.size(); ++i)
    {
        std::cout << MAGENTA << "Message " << (i + 1) << ": " << RESET << messages[i] << std::endl;
        
        for (size_t j = 0; j < loggers.size(); ++j)
        {
            loggers[j]->write(messages[i]);
        }
        
        std::cout << std::endl;
    }

    return 0;
}
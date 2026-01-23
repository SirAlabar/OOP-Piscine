#ifndef ILOGGER_HPP
#define ILOGGER_HPP

#include <string>

class ILogger
{
public:
    virtual void write(const std::string message) = 0;
    virtual ~ILogger() {}
};

#endif
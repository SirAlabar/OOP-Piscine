#ifndef HEADERPROVIDER_HPP
#define HEADERPROVIDER_HPP

#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>

class IHeader
{
public:
    virtual std::string getHeader() const = 0;
    virtual ~IHeader() {}
};

class ConstantHeader : public IHeader
{
private:
    std::string header;

public:
    ConstantHeader(const std::string& h)
        : header(h)
    {
    }

    std::string getHeader() const
    {
        return header;
    }
};

class DateHeader : public IHeader
{
public:
    std::string getHeader() const
    {
        std::time_t now = std::time(0);
        std::tm* t = std::localtime(&now);
        
        std::stringstream ss;
        ss << "[" 
           << (t->tm_year + 1900) << "-"
           << std::setfill('0') << std::setw(2) << (t->tm_mon + 1) << "-"
           << std::setfill('0') << std::setw(2) << t->tm_mday 
           << " "
           << std::setfill('0') << std::setw(2) << t->tm_hour << ":"
           << std::setfill('0') << std::setw(2) << t->tm_min << ":"
           << std::setfill('0') << std::setw(2) << t->tm_sec
           << "] ";
        
        return ss.str();
    }
};

#endif
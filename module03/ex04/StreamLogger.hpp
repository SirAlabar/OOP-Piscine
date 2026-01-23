#ifndef STREAMLOGGER_HPP
#define STREAMLOGGER_HPP

#include "ILogger.hpp"
#include "HeaderProvider.hpp"
#include <iostream>

class StreamLogger : public ILogger
{
private:
    std::ostream* stream;
    IHeader* headerProvider;

public:
    StreamLogger(std::ostream* stream, IHeader* headerProvider = 0)
        : stream(stream), headerProvider(headerProvider)
    {
    }

    void write(std::string message)
    {
        if (headerProvider)
        {
            *stream << headerProvider->getHeader();
        }
        *stream << message << std::endl;
    }
};

#endif
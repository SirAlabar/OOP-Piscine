#ifndef FILELOGGER_HPP
#define FILELOGGER_HPP

#include "ILogger.hpp"
#include "HeaderProvider.hpp"
#include <fstream>

class FileLogger : public ILogger
{
private:
    std::string filename;
    IHeader* headerProvider;

public:
    FileLogger(const std::string& filename, IHeader* headerProvider = 0)
        : filename(filename), headerProvider(headerProvider)
    {
    }

    void write(std::string message)
    {
        std::ofstream file(filename.c_str(), std::ios::app);
        if (file.is_open())
        {
            if (headerProvider)
            {
                file << headerProvider->getHeader();
            }
            file << message << std::endl;
            file.close();
        }
    }
};

#endif
#ifndef ILOGGER_HPP
#define ILOGGER_HPP

#include <string>

// Narrow interface for diagnostic logging.
// Implemented by ConsoleOutputWriter (and any future log sink).
// Inject ILogger* into classes that only need to log progress or errors —
// they should not be forced to depend on the full IOutputWriter interface.
class ILogger
{
public:
    virtual ~ILogger() = default;

    // Informational progress message (e.g. "Parsing network file...").
    virtual void writeProgress(const std::string& message) = 0;

    // Error or warning message.
    virtual void writeError(const std::string& message) = 0;
};

#endif
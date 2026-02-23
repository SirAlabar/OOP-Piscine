#ifndef IRUNMODEHANDLER_HPP
#define IRUNMODEHANDLER_HPP

#include <string>

class CLI;
class RunSession;

class IRunModeHandler
{
public:
    virtual ~IRunModeHandler() = default;

    virtual bool matches(const CLI& cli) const = 0;

    virtual int run(const std::string& netFile,
                    const std::string& trainFile,
                    RunSession& session) = 0;
};

#endif

#ifndef MODEHANDLERS_HPP
#define MODEHANDLERS_HPP

#include "app/IRunModeHandler.hpp"

class MonteCarloModeHandler : public IRunModeHandler
{
public:
    bool matches(const CLI& cli) const override;

    int run(const std::string& netFile,
            const std::string& trainFile,
            RunSession& session) override;
};

class ReplayModeHandler : public IRunModeHandler
{
public:
    bool matches(const CLI& cli) const override;

    int run(const std::string& netFile,
            const std::string& trainFile,
            RunSession& session) override;
};

class HotReloadModeHandler : public IRunModeHandler
{
public:
    bool matches(const CLI& cli) const override;

    int run(const std::string& netFile,
            const std::string& trainFile,
            RunSession& session) override;
};

class RenderModeHandler : public IRunModeHandler
{
public:
    bool matches(const CLI& cli) const override;

    int run(const std::string& netFile,
            const std::string& trainFile,
            RunSession& session) override;
};

class ConsoleModeHandler : public IRunModeHandler
{
public:
    bool matches(const CLI& cli) const override;

    int run(const std::string& netFile,
            const std::string& trainFile,
            RunSession& session) override;
};

#endif

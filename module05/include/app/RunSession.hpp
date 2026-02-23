#ifndef RUNSESSION_HPP
#define RUNSESSION_HPP

#include "simulation/core/SimulationBuilder.hpp"
#include "simulation/core/SimulationManager.hpp"
#include <functional>
#include <memory>
#include <string>

class CLI;
class IOutputWriter;
class FileOutputWriter;
class CommandManager;

class RunSession
{
public:
    RunSession(const CLI& cli,
               IOutputWriter& consoleWriter,
               SimulationManager& sim,
               SimulationBuilder& builder);

    bool buildSimulation(const std::string& netFile,
                         const std::string& trainFile,
                         SimulationBundle& outBundle,
                         int seedOverride = -1);

    void teardownSimulation(SimulationBundle& bundle);

    void finishRun(CommandManager* cmdMgr,
                   SimulationBundle& bundle,
                   const std::string& netFile,
                   const std::string& trainFile);

    std::unique_ptr<CommandManager> setupCommandManager() const;

    bool shouldEnableRoundTrip() const;

    SimulationManager& simulation();
    IOutputWriter& output();
    const CLI& cli() const;

private:
    const CLI&         _cli;
    IOutputWriter&     _consoleWriter;
    SimulationManager& _sim;
    SimulationBuilder& _builder;

    void configureSimulation(SimulationBundle& bundle, int seedOverride);
    void flushFinalSnapshots(const std::vector<FileOutputWriter*>& writers, double currentTime) const;
    void saveRecording(CommandManager* cmdMgr,
                       const std::string& netFile,
                       const std::string& trainFile,
                       unsigned int seed,
                       double stopTime) const;
    unsigned int resolveSeed(int seedOverride) const;
};

#endif

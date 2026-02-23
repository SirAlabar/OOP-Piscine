#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "io/CLI.hpp"
#include "simulation/SimulationBuilder.hpp"
#include "simulation/CollisionAvoidance.hpp"
#include "simulation/SimulationManager.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class IOutputWriter;
class Graph;
class Train;
class FileOutputWriter;
class CommandManager;

// Orchestrates run-mode selection and simulation lifecycle.
// Construction concerns (parsing, validation, train building) are
// delegated to SimulationBuilder.
class Application
{
private:
    CLI                _cli;
    IOutputWriter*     _consoleWriter;
    CollisionAvoidance _collision;
    SimulationManager  _sim;
    SimulationBuilder  _builder;

    // --- Simulation lifecycle ---
    bool _buildSimulation(const std::string& netFile, const std::string& trainFile, SimulationBundle& outBundle, int seedOverride = -1);
    void _teardownSimulation(SimulationBundle& bundle);
    void _configureSimulation(SimulationBundle& bundle, int seedOverride);

    // --- One method per execution mode dispatched from run() ---
    int _runMonteCarlo(const std::string& netFile, const std::string& trainFile);
    int _runReplay(const std::string& netFile, const std::string& trainFile);
    int _runHotReload(const std::string& netFile, const std::string& trainFile);
    int _runRender(const std::string& netFile, const std::string& trainFile);
    int _runConsole(const std::string& netFile, const std::string& trainFile);

    // --- Shared tail: flush + save + teardown ---
    void _finishRun(CommandManager*    cmdMgr,
                    SimulationBundle&  bundle,
                    const std::string& netFile,
                    const std::string& trainFile);

    // --- Hot-reload helpers ---
    bool _validateFilesForReload(const std::string& netFile, const std::string& trainFile) const;
    void _recordReloadCommand(CommandManager* cmdMgr, double reloadTime, const std::string& oldNet, const std::string& oldTrain, const std::string& netFile, const std::string& trainFile, const std::function<bool(const std::string&, const std::string&)>& rebuildCallback) const;

    // --- Mode policy ---
    bool _shouldEnableRoundTrip() const;

    // --- DRY helpers ---
    void            _printConfiguration(const std::string& netFile, const std::string& trainFile) const;
    CommandManager* _setupCommandManager() const;
    void            _flushFinalSnapshots(const std::vector<FileOutputWriter*>& writers, double currentTime) const;
    void            _saveRecording(CommandManager* cmdMgr, const std::string& netFile, const std::string& trainFile, unsigned int seed, double stopTime) const;
    unsigned int    _resolveSeed(int seedOverride) const;
    static std::string _readFile(const std::string& path);

public:
    Application(int argc, char* argv[]);
    ~Application();
    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;

    int run();
};

#endif
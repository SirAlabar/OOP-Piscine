#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "io/CLI.hpp"
#include "patterns/factories/TrainFactory.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class IOutputWriter;
class Graph;
class Train;
class FileOutputWriter;
class CommandManager;
class IPathfindingStrategy;

// Owns the three objects built and torn down together each simulation run.
struct SimulationBundle
{
    Graph*                         graph   = nullptr;
    std::vector<Train*>            trains;
    std::vector<FileOutputWriter*> writers;
};

// Orchestrates the entire railway simulation application.
class Application
{
public:
    Application(int argc, char* argv[]);
    ~Application();
    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;

    int run();

private:
    CLI            _cli;
    IOutputWriter* _consoleWriter;

    // --- Simulation lifecycle ---
    bool _buildSimulation(const std::string& netFile, const std::string& trainFile, SimulationBundle& outBundle, int seedOverride = -1);
    void _teardownSimulation(SimulationBundle& bundle);

    // --- Build sub-steps called only from _buildSimulation ---
    Graph*                                _parseNetwork(const std::string& netFile);
    std::vector<TrainConfig>              _parseTrains(const std::string& trainFile);
    std::unique_ptr<IPathfindingStrategy> _createStrategy() const;
    std::vector<Train*>                   _buildTrains(const std::vector<TrainConfig>& configs, Graph* graph, IPathfindingStrategy* strategy);
    std::vector<FileOutputWriter*>        _createOutputWriters(const std::vector<Train*>& trains);
    void                                  _configureSimulation(SimulationBundle& bundle, int seedOverride);

    // --- One method per execution mode dispatched from run() ---
    int _runMonteCarlo(const std::string& netFile, const std::string& trainFile);
    int _runReplay(const std::string& netFile, const std::string& trainFile);
    int _runHotReload(const std::string& netFile, const std::string& trainFile);
    int _runRender(const std::string& netFile, const std::string& trainFile);
    int _runConsole(const std::string& netFile, const std::string& trainFile);

    // --- Hot-reload helpers ---
    bool _validateFilesForReload(const std::string& netFile, const std::string& trainFile) const;
    void _recordReloadCommand(CommandManager* cmdMgr, double reloadTime, const std::string& oldNet, const std::string& oldTrain, const std::string& netFile, const std::string& trainFile, const std::function<bool(const std::string&, const std::string&)>& rebuildCallback) const;

    // --- Mode policy ---
    bool _shouldEnableRoundTrip() const;

    // --- Phase 1 DRY helpers ---
    void            _printConfiguration(const std::string& netFile, const std::string& trainFile) const;
    CommandManager* _setupCommandManager() const;
    void            _flushFinalSnapshots(const std::vector<FileOutputWriter*>& writers, double currentTime) const;
    void            _saveRecording(CommandManager* cmdMgr, const std::string& netFile, const std::string& trainFile, unsigned int seed) const;
    unsigned int    _resolveSeed(int seedOverride) const;
    static double   _estimateJourneyMinutes(const Train* train);
    static std::string _readFile(const std::string& path);
};

#endif
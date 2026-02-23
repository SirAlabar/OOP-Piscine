#ifndef SIMULATIONBUILDER_HPP
#define SIMULATIONBUILDER_HPP

#include "core/Node.hpp"
#include "core/Train.hpp"
#include "patterns/creational/factories/TrainFactory.hpp"
#include <memory>
#include <string>
#include <vector>

class Graph;
class Train;
class FileOutputWriter;
class IOutputWriter;
class IPathfindingStrategy;

// Owns the three objects built and torn down together each simulation run.
struct SimulationBundle
{
    Graph*                         graph   = nullptr;
    std::vector<Train*>            trains;
    std::vector<FileOutputWriter*> writers;
};

// Per-config result returned by validateTrainConfigs.
// Status is set explicitly — callers must not infer state from other fields.
struct TrainValidationResult
{
    enum class Status { InvalidConfig, NoPath, Routable };

    Status      status = Status::InvalidConfig;
    TrainConfig config;
    std::string error;

    // Valid only when status == Routable.
    std::vector<PathSegment> path;
};

// Build a SimulationBundle from config files.
// Owns parsing, validation, train construction, and writer creation.
// Application uses this to separate construction from run-mode orchestration.
class SimulationBuilder
{
private:
    IOutputWriter* _logger;
    std::string    _pathfindingAlgo;

    Graph*                                _parseNetwork(const std::string& netFile);
    std::vector<TrainConfig>              _parseTrains(const std::string& trainFile);
    std::unique_ptr<IPathfindingStrategy> _createStrategy() const;
    std::vector<Train*>                   _buildTrains(const std::vector<TrainValidationResult>& results, Graph* graph);
    std::vector<FileOutputWriter*>        _createOutputWriters(const std::vector<Train*>& trains);

    static double _estimateJourneyMinutes(const Train* train);

public:
    SimulationBuilder(IOutputWriter* logger, const std::string& pathfindingAlgo);

    // Full pipeline: parse network + trains, validate, build trains, create writers.
    // Throws on parse/IO failure. Returns populated bundle on success.
    SimulationBundle build(const std::string& netFile, const std::string& trainFile);

    // Stateless validation helper — exposed for hot-reload pre-flight checks.
    // Does NOT log — caller handles per-context message prefixes.
    static std::vector<TrainValidationResult> validateTrainConfigs(
        const std::vector<TrainConfig>& configs,
        Graph*                          graph,
        IPathfindingStrategy*           strategy);
};

#endif
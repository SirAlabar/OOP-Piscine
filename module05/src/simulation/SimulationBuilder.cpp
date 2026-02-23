#include "simulation/SimulationBuilder.hpp"
#include "io/RailNetworkParser.hpp"
#include "io/TrainConfigParser.hpp"
#include "io/FileOutputWriter.hpp"
#include "io/IOutputWriter.hpp"
#include "patterns/factories/TrainFactory.hpp"
#include "patterns/factories/TrainValidator.hpp"
#include "patterns/strategies/IPathfindingStrategy.hpp"
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "patterns/strategies/AStarStrategy.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "core/Train.hpp"
#include <memory>

SimulationBuilder::SimulationBuilder(IOutputWriter* logger, const std::string& pathfindingAlgo)
    : _logger(logger),
      _pathfindingAlgo(pathfindingAlgo)
{
}

SimulationBundle SimulationBuilder::build(
    const std::string& netFile,
    const std::string& trainFile)
{
    SimulationBundle bundle;

    bundle.graph = _parseNetwork(netFile);

    std::vector<TrainConfig>              configs  = _parseTrains(trainFile);
    std::unique_ptr<IPathfindingStrategy> strategy = _createStrategy();

    std::vector<TrainValidationResult> results =
        validateTrainConfigs(configs, bundle.graph, strategy.get());

    bundle.trains  = _buildTrains(results, bundle.graph);
    bundle.writers = _createOutputWriters(bundle.trains);

    return bundle;
}

std::vector<TrainValidationResult> SimulationBuilder::validateTrainConfigs(
    const std::vector<TrainConfig>& configs,
    Graph*                          graph,
    IPathfindingStrategy*           strategy)
{
    std::vector<TrainValidationResult> results;
    results.reserve(configs.size());

    for (const TrainConfig& config : configs)
    {
        TrainValidationResult result;
        result.config = config;

        ValidationResult vr = TrainValidator::validate(config, graph);
        if (!vr.valid)
        {
            result.status = TrainValidationResult::Status::InvalidConfig;
            result.error  = vr.error;
            results.push_back(result);
            continue;
        }

        Node* src  = graph->getNode(config.departureStation);
        Node* dst  = graph->getNode(config.arrivalStation);
        auto  path = strategy->findPath(graph, src, dst);

        if (path.empty())
        {
            result.status = TrainValidationResult::Status::NoPath;
            result.error  = "No path from " + config.departureStation +
                            " to "           + config.arrivalStation;
            results.push_back(result);
            continue;
        }

        result.status = TrainValidationResult::Status::Routable;
        result.path   = std::move(path);
        results.push_back(result);
    }

    return results;
}

Graph* SimulationBuilder::_parseNetwork(const std::string& netFile)
{
    _logger->writeProgress("Parsing network file...");
    RailNetworkParser parser(netFile);
    Graph*            graph = parser.parse();
    _logger->writeGraphDetails(graph->getNodes(), graph->getRails());
    _logger->writeNetworkSummary(graph->getNodeCount(), graph->getRailCount());
    return graph;
}

std::vector<TrainConfig> SimulationBuilder::_parseTrains(const std::string& trainFile)
{
    _logger->writeProgress("Parsing train file...");
    TrainConfigParser        parser(trainFile);
    std::vector<TrainConfig> configs = parser.parse();
    _logger->writeProgress(std::to_string(configs.size()) + " trains parsed");
    return configs;
}

std::unique_ptr<IPathfindingStrategy> SimulationBuilder::_createStrategy() const
{
    if (_pathfindingAlgo == "astar")
    {
        _logger->writeProgress("Using A* pathfinding");
        return std::make_unique<AStarStrategy>();
    }
    _logger->writeProgress("Using Dijkstra pathfinding");
    return std::make_unique<DijkstraStrategy>();
}

std::vector<Train*> SimulationBuilder::_buildTrains(
    const std::vector<TrainValidationResult>& results,
    Graph*                                    graph)
{
    _logger->writeProgress("Creating trains and finding paths...");
    std::vector<Train*> trains;

    for (const TrainValidationResult& result : results)
    {
        if (result.status == TrainValidationResult::Status::InvalidConfig)
        {
            _logger->writeError(result.error);
            continue;
        }

        if (result.status == TrainValidationResult::Status::NoPath)
        {
            _logger->writeError(
                "No path found for train " + result.config.name +
                " from " + result.config.departureStation +
                " to "   + result.config.arrivalStation);
            continue;
        }

        Train* train = TrainFactory::create(result.config, graph);
        if (!train)
        {
            _logger->writeError("Failed to create train: " + result.config.name);
            continue;
        }

        train->setPath(result.path);
        _logger->writePathDebug(train);
        trains.push_back(train);
        _logger->writeTrainCreated(
            train->getName(), train->getID(),
            result.config.departureStation, result.config.arrivalStation,
            static_cast<int>(result.path.size()));
    }

    return trains;
}

std::vector<FileOutputWriter*> SimulationBuilder::_createOutputWriters(
    const std::vector<Train*>& trains)
{
    _logger->writeProgress("Creating output files...");
    std::vector<FileOutputWriter*> writers;

    for (Train* train : trains)
    {
        double estMinutes = _estimateJourneyMinutes(train);

        std::unique_ptr<FileOutputWriter> writer(new FileOutputWriter(train));
        writer->open();
        writer->writeHeader(estMinutes);
        writer->writePathInfo();
        writers.push_back(writer.release());

        _logger->writeProgress(
            "Created: output/" + train->getName() + "_" +
            train->getDepartureTime().toString() + ".result" +
            " (estimated: " + std::to_string(static_cast<int>(estMinutes)) + " min)");
    }

    return writers;
}

double SimulationBuilder::_estimateJourneyMinutes(const Train* train)
{
    double estMinutes = 0.0;
    for (const PathSegment& seg : train->getPath())
    {
        estMinutes += (seg.rail->getLength() / seg.rail->getSpeedLimit()) * 60.0;
    }
    return estMinutes;
}
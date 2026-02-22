#include "analysis/MonteCarloRunner.hpp"
#include "io/RailNetworkParser.hpp"
#include "io/TrainConfigParser.hpp"
#include "io/ILogger.hpp"
#include "patterns/factories/TrainFactory.hpp"
#include "patterns/factories/TrainValidator.hpp"
#include "patterns/strategies/IPathfindingStrategy.hpp"
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "patterns/strategies/AStarStrategy.hpp"
#include "simulation/SimulationConfig.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "patterns/states/ITrainState.hpp"
#include "utils/FileSystemUtils.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

MonteCarloRunner::MonteCarloRunner(const std::string& networkFile,
                                   const std::string& trainFile,
                                   unsigned int       baseSeed,
                                   unsigned int       numRuns,
                                   const std::string& pathfindingAlgo,
                                   ILogger*           logger)
    : _networkFile(networkFile),
      _trainFile(trainFile),
      _baseSeed(baseSeed),
      _numRuns(numRuns),
      _pathfindingAlgo(pathfindingAlgo),
      _logger(logger)
{
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void MonteCarloRunner::runAll()
{
    _allMetrics.clear();
    _allMetrics.reserve(_numRuns);

    for (unsigned int i = 0; i < _numRuns; ++i)
    {
        unsigned int seed = _baseSeed + i;
        log("Run " + std::to_string(i + 1) + "/" + std::to_string(_numRuns)
            + " (seed=" + std::to_string(seed) + ")");

        SimulationMetrics metrics = runSingleSimulation(seed);
        _allMetrics.push_back(metrics);
    }

    log("Monte Carlo complete: " + std::to_string(_numRuns) + " runs finished.");
}

void MonteCarloRunner::writeCSV(const std::string& filename) const
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open CSV output: " + filename);
    }

    // Header
    file << "seed,totalDuration,totalEvents,collisionAvoidances";

    // Collect all unique train names across all runs for column headers
    std::vector<std::string> trainNames;
    for (const SimulationMetrics& m : _allMetrics)
    {
        for (const auto& kv : m.trainMetrics)
        {
            const std::string& name = kv.first;
            bool found = false;
            for (const std::string& n : trainNames)
            {
                if (n == name)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                trainNames.push_back(name);
            }
        }
    }

    for (const std::string& name : trainNames)
    {
        file << "," << name << "_actualTime"
             << "," << name << "_estimatedTime"
             << "," << name << "_stateTransitions"
             << "," << name << "_eventsAffecting"
             << "," << name << "_reached";
    }
    file << "\n";

    // Rows
    for (const SimulationMetrics& m : _allMetrics)
    {
        file << m.seed
             << "," << m.totalDuration
             << "," << m.totalEventsGenerated
             << "," << m.collisionAvoidanceActivations;

        for (const std::string& name : trainNames)
        {
            auto it = m.trainMetrics.find(name);
            if (it != m.trainMetrics.end())
            {
                const TrainMetrics& tm = it->second;
                file << "," << tm.actualTravelTime
                     << "," << tm.estimatedTravelTime
                     << "," << tm.stateTransitions
                     << "," << tm.eventsAffectingTrain
                     << "," << (tm.reachedDestination ? 1 : 0);
            }
            else
            {
                file << ",,,,,";
            }
        }
        file << "\n";
    }

    log("CSV written: " + filename);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void MonteCarloRunner::log(const std::string& message) const
{
    if (_logger)
    {
        _logger->writeProgress(message);
    }
}

double MonteCarloRunner::estimateJourneySeconds(const Train* train)
{
    if (!train)
    {
        return 0.0;
    }

    double seconds = 0.0;
    for (const PathSegment& seg : train->getPath())
    {
        if (seg.rail && seg.rail->getSpeedLimit() > 0.0)
        {
            seconds += (seg.rail->getLength() / seg.rail->getSpeedLimit())
                       * SimConfig::SECONDS_PER_HOUR;
        }
    }
    return seconds;
}

Graph* MonteCarloRunner::parseNetwork() const
{
    RailNetworkParser parser(_networkFile);
    return parser.parse();
}

std::vector<TrainConfig> MonteCarloRunner::parseTrains() const
{
    TrainConfigParser parser(_trainFile);
    return parser.parse();
}

IPathfindingStrategy* MonteCarloRunner::selectStrategy(
    IPathfindingStrategy& dijkstra,
    IPathfindingStrategy& astar)
{
    if (_pathfindingAlgo == "astar")
    {
        return &astar;
    }
    return &dijkstra;
}

std::vector<Train*> MonteCarloRunner::buildTrains(
    Graph*                graph,
    StatsCollector&       stats,
    IPathfindingStrategy* strategy)
{
    std::vector<TrainConfig> configs = parseTrains();
    std::vector<Train*>      trains;

    for (const TrainConfig& config : configs)
    {
        // Validate config against the network (mirrors Application::_buildTrains).
        ValidationResult vr = TrainValidator::validate(config, graph);
        if (!vr.valid)
        {
            log("Skipping train '" + config.name + "': " + vr.error);
            continue;
        }

        Train* train = TrainFactory::create(config, graph);
        if (!train)
        {
            log("Failed to create train: " + config.name);
            continue;
        }

        Node* startNode = graph->getNode(config.departureStation);
        Node* endNode   = graph->getNode(config.arrivalStation);
        auto  path      = strategy->findPath(graph, startNode, endNode);

        if (path.empty())
        {
            log("No path for train '" + config.name + "' ("
                + config.departureStation + " -> " + config.arrivalStation + ")");
            delete train;
            continue;
        }

        train->setPath(path);

        double estimatedSeconds = estimateJourneySeconds(train);
        stats.registerTrain(train, estimatedSeconds);

        trains.push_back(train);
    }

    return trains;
}

void MonteCarloRunner::setupSimulation(
    Graph*                     graph,
    const std::vector<Train*>& trains,
    StatsCollector&            stats,
    unsigned int               seed)
{
    _sim.reset();

    SimulationConfig config;
    config.network   = graph;
    config.seed      = seed;
    config.roundTrip = false;
    config.writer    = nullptr;  // Silent — no console output during MC runs.

    _sim.configure(config);
    _sim.setStatsCollector(&stats);

    for (Train* train : trains)
    {
        _sim.addTrain(train);
    }
}

void MonteCarloRunner::runSimulationLoop(
    const std::vector<Train*>&  trains,
    StatsCollector&             stats,
    std::map<Train*, double>&   departureTime,
    std::map<Train*, double>&   arrivalTime)
{
    // Track previous states to detect transitions.
    std::map<Train*, std::string> prevStates;
    for (Train* train : trains)
    {
        if (train)
        {
            prevStates[train] = "Idle";
        }
    }

    _sim.start();

    constexpr double maxSimTime = 1e9;  // shouldStopEarly() terminates when done.

    while (_sim.isRunning() && _sim.getCurrentTime() < maxSimTime)
    {
        _sim.step();

        double currentTime = _sim.getCurrentTime();

        for (Train* train : trains)
        {
            if (!train)
            {
                continue;
            }

            if (!train->getCurrentState())
            {
                continue;
            }

            std::string currentStateName = train->getCurrentState()->getName();
            std::string& prevStateName   = prevStates[train];

            if (currentStateName != prevStateName)
            {
                // Detect departure (Idle → anything else)
                if (prevStateName == "Idle" && currentStateName != "Idle")
                {
                    departureTime[train] = currentTime;
                }

                // Detect arrival (transition to finished state at destination)
                if (train->isFinished() && arrivalTime.find(train) == arrivalTime.end())
                {
                    arrivalTime[train] = currentTime;
                    stats.checkTrainDestination(train);
                }

                // Collision avoidance proxy: Idle → Waiting mid-journey
                if (currentStateName == "Waiting" && prevStateName != "Idle")
                {
                    stats.recordCollisionAvoidance();
                }

                stats.recordStateTransition(train, prevStateName, currentStateName);
                prevStateName = currentStateName;
            }

            // Check arrival for already-finished trains that may not change state
            if (train->isFinished() && arrivalTime.find(train) == arrivalTime.end())
            {
                arrivalTime[train] = currentTime;
                stats.checkTrainDestination(train);
            }
        }

        // shouldStopEarly is called inside sim.run(), but since we drive via
        // step() here we replicate the stop condition manually.
        bool allDone = true;
        for (Train* train : trains)
        {
            if (train && !train->isFinished())
            {
                allDone = false;
                break;
            }
        }

        if (allDone)
        {
            break;
        }
    }
}

SimulationMetrics MonteCarloRunner::finalizeMetrics(
    StatsCollector&                    stats,
    const std::vector<Train*>&         trains,
    const std::map<Train*, double>&    departureTime,
    const std::map<Train*, double>&    arrivalTime)
{
    double totalDuration = _sim.getCurrentTime();
    stats.finalize(totalDuration);

    // Patch actual travel times from our timing maps.
    for (Train* train : trains)
    {
        if (!train)
        {
            continue;
        }

        auto deptIt = departureTime.find(train);
        auto arrIt  = arrivalTime.find(train);

        if (deptIt != departureTime.end() && arrIt != arrivalTime.end())
        {
            double actual = arrIt->second - deptIt->second;

            SimulationMetrics& m = const_cast<SimulationMetrics&>(stats.getMetrics());
            auto tmIt = m.trainMetrics.find(train->getName());
            if (tmIt != m.trainMetrics.end())
            {
                tmIt->second.actualTravelTime = actual;
            }
        }
    }

    return stats.getMetrics();
}

void MonteCarloRunner::cleanup(Graph* graph, std::vector<Train*>& trains)
{
    _sim.reset();

    for (Train* train : trains)
    {
        delete train;
    }
    trains.clear();

    delete graph;
}

SimulationMetrics MonteCarloRunner::runSingleSimulation(unsigned int seed)
{
    DijkstraStrategy dijkstra;
    AStarStrategy    astar;

    Graph*           graph  = parseNetwork();
    StatsCollector   stats(seed);

    IPathfindingStrategy* strategy = selectStrategy(dijkstra, astar);
    std::vector<Train*>   trains   = buildTrains(graph, stats, strategy);

    if (trains.empty())
    {
        cleanup(graph, trains);
        SimulationMetrics empty{};
        empty.seed = seed;
        return empty;
    }

    setupSimulation(graph, trains, stats, seed);

    std::map<Train*, double> departureTime;
    std::map<Train*, double> arrivalTime;

    runSimulationLoop(trains, stats, departureTime, arrivalTime);

    SimulationMetrics metrics =
        finalizeMetrics(stats, trains, departureTime, arrivalTime);

    cleanup(graph, trains);
    return metrics;
}
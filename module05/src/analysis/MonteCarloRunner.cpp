#include "analysis/MonteCarloRunner.hpp"
#include "analysis/StatsCollector.hpp"
#include "io/RailNetworkParser.hpp"
#include "io/TrainConfigParser.hpp"
#include "patterns/factories/TrainFactory.hpp"
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "patterns/strategies/AStarStrategy.hpp"
#include "simulation/SimulationContext.hpp"
#include "patterns/states/ITrainState.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "utils/FileSystemUtils.hpp"
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <iomanip>

MonteCarloRunner::MonteCarloRunner(const std::string& networkFile,
                                   const std::string& trainFile,
                                   unsigned int baseSeed,
                                   unsigned int numRuns,
                                   const std::string& pathfindingAlgo)
    : _networkFile(networkFile),
      _trainFile(trainFile),
      _baseSeed(baseSeed),
      _numRuns(numRuns),
      _pathfindingAlgo(pathfindingAlgo)
{
}

void MonteCarloRunner::runAll()
{
    _allMetrics.clear();

    std::cout << "\n========================================\n";
    std::cout << "  MONTE CARLO SIMULATION\n";
    std::cout << "========================================\n";
    std::cout << "Runs: " << _numRuns << "\n";
    std::cout << "Base seed: " << _baseSeed << "\n";
    std::cout << "Pathfinding: " << _pathfindingAlgo << "\n\n";

    for (unsigned int i = 0; i < _numRuns; ++i)
    {
        unsigned int seed = _baseSeed + i;

        std::cout << "Run " << (i + 1) << "/" << _numRuns
                  << " (seed=" << seed << ")... ";
        std::cout.flush();

        SimulationMetrics metrics = runSingleSimulation(seed);
        _allMetrics.push_back(metrics);

        std::cout << "done (" << std::fixed << std::setprecision(1)
                  << metrics.totalDuration << "s)\n";
    }

    std::cout << "\nAll simulations complete!\n\n";
}

void MonteCarloRunner::writeCSV(const std::string& filename) const
{
    FileSystemUtils::ensureOutputDirectoryExists();

    std::ofstream file(filename);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open CSV file: " + filename);
    }

    // Header
    file << "Run,Seed,TotalDuration(s),TotalEvents,CollisionAvoidances";

    if (!_allMetrics.empty())
    {
        for (const auto& pair : _allMetrics[0].trainMetrics)
        {
            const std::string& name = pair.first;
            file << "," << name << "_ActualTime(s)"
                 << "," << name << "_EstimatedTime(s)"
                 << "," << name << "_Transitions"
                 << "," << name << "_Waits"
                 << "," << name << "_Emergencies"
                 << "," << name << "_Events"
                 << "," << name << "_Completed";
        }
    }

    file << "\n";

    // Rows
    for (size_t i = 0; i < _allMetrics.size(); ++i)
    {
        const SimulationMetrics& m = _allMetrics[i];

        file << (i + 1)
             << "," << m.seed
             << "," << std::fixed << std::setprecision(2) << m.totalDuration
             << "," << m.totalEventsGenerated
             << "," << m.collisionAvoidanceActivations;

        if (!_allMetrics.empty())
        {
            for (const auto& pair : _allMetrics[0].trainMetrics)
            {
                const std::string& name = pair.first;
                auto it = m.trainMetrics.find(name);

                if (it != m.trainMetrics.end())
                {
                    const TrainMetrics& tm = it->second;
                    file << "," << tm.actualTravelTime
                         << "," << tm.estimatedTravelTime
                         << "," << tm.stateTransitions
                         << "," << tm.waitEvents
                         << "," << tm.emergencyEvents
                         << "," << tm.eventsAffectingTrain
                         << "," << (tm.reachedDestination ? 1 : 0);
                }
                else
                {
                    file << ",0,0,0,0,0,0,0";
                }
            }
        }

        file << "\n";
    }

    file.close();

    std::cout << "✓ Statistics written to: " << filename << "\n";
    std::cout << "  Total runs: " << _allMetrics.size() << "\n\n";
}

SimulationMetrics MonteCarloRunner::runSingleSimulation(unsigned int seed)
{
    StatsCollector stats(seed);
    Graph* graph = parseNetwork();

    DijkstraStrategy dijkstra;
    AStarStrategy    astar;

    IPathfindingStrategy* strategy = selectStrategy(dijkstra, astar);
    std::vector<Train*> trains = buildTrains(graph, stats, strategy);

    setupSimulation(graph, trains, stats, seed);

    std::map<Train*, double> departureTime;
    std::map<Train*, double> arrivalTime;

    runSimulationLoop(trains, stats, departureTime, arrivalTime);

    SimulationMetrics metrics = finalizeMetrics(stats, trains, departureTime, arrivalTime);

    cleanup(graph, trains);

    return metrics;
}

IPathfindingStrategy* MonteCarloRunner::selectStrategy(
    IPathfindingStrategy& dijkstra, IPathfindingStrategy& astar)
{
    if (_pathfindingAlgo == "astar")
    {
        return &astar;
    }
    return &dijkstra;
}

std::vector<Train*> MonteCarloRunner::buildTrains(
    Graph* graph, StatsCollector& stats, IPathfindingStrategy* strategy)
{
    std::vector<Train*> trains;
    std::vector<TrainConfig> configs = parseTrains();

    for (const TrainConfig& config : configs)
    {
        Train* train = TrainFactory::create(config, graph);

        if (!train)
        {
            continue;
        }

        Node* start = graph->getNode(config.departureStation);
        Node* end   = graph->getNode(config.arrivalStation);

        if (!start || !end)
        {
            delete train;
            continue;
        }

        auto path = strategy->findPath(graph, start, end);

        if (path.empty())
        {
            delete train;
            continue;
        }

        train->setPath(path);

        double estimatedSeconds = 0.0;
        for (const PathSegment& segment : path)
        {
            double hours = segment.rail->getLength() / segment.rail->getSpeedLimit();
            estimatedSeconds += hours * 3600.0;
        }

        stats.registerTrain(train, estimatedSeconds);
        trains.push_back(train);
    }

    return trains;
}

void MonteCarloRunner::setupSimulation(Graph* graph,
                                       const std::vector<Train*>& trains,
                                       StatsCollector& stats,
                                       unsigned int seed)
{
    _sim.reset();
    _sim.setSimulationWriter(nullptr);
    _sim.setEventSeed(seed);
    _sim.setNetwork(graph);
    _sim.setStatsCollector(&stats);

    for (Train* train : trains)
    {
        _sim.addTrain(train);
    }

    _sim.start();
}

void MonteCarloRunner::runSimulationLoop(const std::vector<Train*>& trains,
                                         StatsCollector& stats,
                                         std::map<Train*, double>& departureTime,
                                         std::map<Train*, double>& arrivalTime)
{
    SimulationContext* context = _sim.getContext();

    if (!context)
    {
        throw std::runtime_error("SimulationContext null");
    }

    StateRegistry& states = context->states();

    std::map<Train*, ITrainState*> previousStates;
    for (Train* train : trains)
    {
        previousStates[train] = train->getCurrentState();
    }

    const double maxTime = SimConfig::SECONDS_PER_DAY;

    while (_sim.isRunning() && _sim.getCurrentTime() < maxTime)
    {
        for (Train* train : trains)
        {
            if (!train)
            {
                continue;
            }

            ITrainState*  current  = train->getCurrentState();
            ITrainState*& previous = previousStates[train];

            if (current != previous)
            {
                std::string prevName = previous ? previous->getName() : "";
                std::string currName = current  ? current->getName()  : "";

                stats.recordStateTransition(train, prevName, currName);

                if (previous == states.idle() && current != states.idle())
                {
                    departureTime[train] = _sim.getCurrentTime();
                }

                if (current == states.waiting() || current == states.emergency())
                {
                    stats.recordCollisionAvoidance();
                }

                previous = current;
            }
        }

        _sim.step();

        bool allFinished = true;
        for (Train* train : trains)
        {
            if (!train)
            {
                continue;
            }

            if (!train->isFinished())
            {
                allFinished = false;
            }
            else if (arrivalTime.find(train) == arrivalTime.end())
            {
                arrivalTime[train] = _sim.getCurrentTime();
            }
        }

        if (allFinished)
        {
            break;
        }
    }
}

SimulationMetrics MonteCarloRunner::finalizeMetrics(
    StatsCollector& stats,
    const std::vector<Train*>& trains,
    const std::map<Train*, double>& departureTime,
    const std::map<Train*, double>& arrivalTime)
{
    stats.finalize(_sim.getCurrentTime());

    // Record total events generated this run.
    int totalEvents = _sim.getTotalEventsGenerated();
    for (int i = 0; i < totalEvents; ++i)
    {
        stats.recordEventGenerated();
    }

    SimulationMetrics metrics = stats.getMetrics();

    for (Train* train : trains)
    {
        if (!train)
        {
            continue;
        }

        stats.checkTrainDestination(train);

        auto dep = departureTime.find(train);
        auto arr = arrivalTime.find(train);

        if (dep != departureTime.end() && arr != arrivalTime.end())
        {
            double actual = arr->second - dep->second;
            metrics.trainMetrics[train->getName()].actualTravelTime = actual;
        }

        metrics.trainMetrics[train->getName()].reachedDestination = train->isFinished();
    }

    return metrics;
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
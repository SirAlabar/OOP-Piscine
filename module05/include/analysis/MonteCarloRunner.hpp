#ifndef MONTE_CARLO_RUNNER_HPP
#define MONTE_CARLO_RUNNER_HPP

#include "analysis/StatsCollector.hpp"

#include <string>
#include <vector>
#include <map>

class Graph;
class Train;
class Node;
class SimulationContext;
class ITrainState;
class IPathfindingStrategy;
struct TrainConfig;

class MonteCarloRunner
{
private:

    std::string  _networkFile;
    std::string  _trainFile;
    unsigned int _baseSeed;
    unsigned int _numRuns;
    std::string  _pathfindingAlgo;

    std::vector<SimulationMetrics> _allMetrics;

public:

    MonteCarloRunner(const std::string& networkFile,
                     const std::string& trainFile,
                     unsigned int baseSeed,
                     unsigned int numRuns,
                     const std::string& pathfindingAlgo);

    void runAll();
    void writeCSV(const std::string& filename) const;

private:

    SimulationMetrics runSingleSimulation(unsigned int seed);
    Graph* parseNetwork() const;
    std::vector<TrainConfig> parseTrains() const;

    IPathfindingStrategy* selectStrategy(IPathfindingStrategy& dijkstra,
        IPathfindingStrategy& astar);

    std::vector<Train*> buildTrains(Graph* graph, StatsCollector& stats,
        IPathfindingStrategy* strategy);

    void setupSimulation(Graph* graph, const std::vector<Train*>& trains,
        StatsCollector& stats, unsigned int seed);

    void runSimulationLoop(const std::vector<Train*>& trains,
        StatsCollector& stats, std::map<Train*, double>& departureTime,
        std::map<Train*, double>& arrivalTime);

    SimulationMetrics finalizeMetrics(StatsCollector& stats,
        const std::vector<Train*>& trains,
        const std::map<Train*, double>& departureTime,
        const std::map<Train*, double>& arrivalTime);

    void cleanup(Graph* graph, std::vector<Train*>& trains);
};

#endif

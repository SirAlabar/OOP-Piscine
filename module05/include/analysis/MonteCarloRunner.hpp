#ifndef MONTE_CARLO_RUNNER_HPP
#define MONTE_CARLO_RUNNER_HPP

#include "analysis/StatsCollector.hpp"
#include <string>
#include <vector>

class Graph;
class IPathfindingStrategy;
struct TrainConfig;

// Runs multiple simulations with different seeds and collects statistics
class MonteCarloRunner
{
private:
	std::string _networkFile;
	std::string _trainFile;
	unsigned int _baseSeed;
	unsigned int _numRuns;
	std::string _pathfindingAlgo;
	
	std::vector<SimulationMetrics> _allMetrics;

public:
	MonteCarloRunner(const std::string& networkFile,
	                 const std::string& trainFile,
	                 unsigned int baseSeed,
	                 unsigned int numRuns,
	                 const std::string& pathfindingAlgo);
	
	// Run all simulations
	void runAll();
	
	// Write results to CSV
	void writeCSV(const std::string& filename) const;

private:
	// Run single simulation with given seed
	SimulationMetrics runSingleSimulation(unsigned int seed);
	
	// Parse network and train configs (reused across runs)
	Graph* parseNetwork() const;
	std::vector<TrainConfig> parseTrains() const;
};

#endif
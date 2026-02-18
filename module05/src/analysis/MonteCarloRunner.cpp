#include "analysis/MonteCarloRunner.hpp"
#include "analysis/StatsCollector.hpp"
#include "io/RailNetworkParser.hpp"
#include "io/TrainConfigParser.hpp"
#include "patterns/factories/TrainFactory.hpp"
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "patterns/strategies/AStarStrategy.hpp"
#include "patterns/observers/EventManager.hpp"
#include "patterns/factories/EventFactory.hpp"
#include "simulation/SimulationContext.hpp"
#include "patterns/states/ITrainState.hpp"
#include "simulation/SimulationManager.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "utils/FileSystemUtils.hpp"
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
	std::cout << "\n========================================\n";
	std::cout << "  MONTE CARLO SIMULATION\n";
	std::cout << "========================================\n";
	std::cout << "Runs: " << _numRuns << "\n";
	std::cout << "Base seed: " << _baseSeed << "\n";
	std::cout << "Pathfinding: " << _pathfindingAlgo << "\n\n";
	
	for (unsigned int i = 0; i < _numRuns; ++i)
	{
		unsigned int seed = _baseSeed + i;
		std::cout << "Run " << (i + 1) << "/" << _numRuns << " (seed=" << seed << ")... ";
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
	// Ensure output directory exists (DRY - shared utility)
	FileSystemUtils::ensureOutputDirectoryExists();
	
	std::ofstream file(filename);
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open CSV file: " + filename);
	}
	
	// Write CSV header
	file << "Run,Seed,TotalDuration(s),TotalEvents,CollisionAvoidances";
	
	// Get train names from first run
	if (!_allMetrics.empty() && !_allMetrics[0].trainMetrics.empty())
	{
		for (const auto& pair : _allMetrics[0].trainMetrics)
		{
			const std::string& trainName = pair.first;
			file << "," << trainName << "_ActualTime(s)"
			     << "," << trainName << "_EstimatedTime(s)"
			     << "," << trainName << "_Transitions"
			     << "," << trainName << "_Waits"
			     << "," << trainName << "_Emergencies"
			     << "," << trainName << "_Events"
			     << "," << trainName << "_Completed";
		}
	}
	file << "\n";
	
	// Write data rows
	for (size_t i = 0; i < _allMetrics.size(); ++i)
	{
		const SimulationMetrics& metrics = _allMetrics[i];
		
		file << (i + 1) << ","
		     << metrics.seed << ","
		     << std::fixed << std::setprecision(2)
		     << metrics.totalDuration << ","
		     << metrics.totalEventsGenerated << ","
		     << metrics.collisionAvoidanceActivations;
		
		// Write per-train metrics (in consistent order)
		if (!_allMetrics.empty() && !_allMetrics[0].trainMetrics.empty())
		{
			for (const auto& pair : _allMetrics[0].trainMetrics)
			{
				const std::string& trainName = pair.first;
				
				// Find this train's metrics in current run
				auto it = metrics.trainMetrics.find(trainName);
				if (it != metrics.trainMetrics.end())
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
					// Train not found - write zeros
					file << ",0,0,0,0,0,0";
				}
			}
		}
		
		file << "\n";
	}
	
	file.close();
	
	std::cout << "✓ Statistics written to: " << filename << "\n";
	std::cout << "  Total runs: " << _allMetrics.size() << "\n";
	std::cout << "  Columns: " << (5 + (_allMetrics.empty() ? 0 : _allMetrics[0].trainMetrics.size() * 6)) << "\n\n";
}

SimulationMetrics MonteCarloRunner::runSingleSimulation(unsigned int seed)
{
	// Create stats collector
	StatsCollector stats(seed);
	
	// Parse network and trains
	Graph* graph = parseNetwork();
	std::vector<TrainConfig> trainConfigs = parseTrains();
	
	// Select pathfinding strategy
	IPathfindingStrategy* strategy = nullptr;
	DijkstraStrategy dijkstra;
	AStarStrategy astar;
	
	if (_pathfindingAlgo == "astar")
	{
		strategy = &astar;
	}
	else
	{
		strategy = &dijkstra;
	}
	
	// Create trains and find paths
	std::vector<Train*> trains;

	for (const auto& config : trainConfigs)
	{
		Train* train = TrainFactory::create(config, graph);
		if (!train)
		{
			continue;
		}
		
		Node* startNode = graph->getNode(config.departureStation);
		Node* endNode = graph->getNode(config.arrivalStation);
		
		if (!startNode || !endNode)
		{
			delete train;
			continue;
		}
		
		auto path = strategy->findPath(graph, startNode, endNode);
		if (path.empty())
		{
			delete train;
			continue;
		}
		
		train->setPath(path);
		trains.push_back(train);
		
		// Calculate estimated time and register with stats
		double estimatedTimeSeconds = 0.0;
		for (const PathSegment& segment : path)
		{
			double segmentTimeHours = segment.rail->getLength() / segment.rail->getSpeedLimit();
			estimatedTimeSeconds += segmentTimeHours * 3600.0;  // Convert to seconds
		}
		stats.registerTrain(train, estimatedTimeSeconds);
	}
	
	// Set up simulation
	SimulationManager& sim = SimulationManager::getInstance();
	sim.reset();
	sim.setSimulationWriter(nullptr);  // No console output in Monte Carlo
	sim.setEventSeed(seed);
	sim.setNetwork(graph);
	sim.setStatsCollector(&stats);  // Enable per-train event tracking
	
	for (Train* train : trains)
	{
		sim.addTrain(train);
	}
	
	// Track state transitions and metrics
	std::map<Train*, ITrainState*> previousStates;
	for (Train* train : trains)
    {
        previousStates[train] = train->getCurrentState();
    }
	
	std::map<Train*, double> departureTime;  // Track when each train departs
	std::map<Train*, double> arrivalTime;    // Track when each train arrives
	
	// Run simulation
	sim.start();
    SimulationContext* context = sim.getContext();
    if (!context)
    {
        throw std::runtime_error("SimulationContext is null in MonteCarloRunner");
    }
    StateRegistry& states = context->states();

	double maxTime = SimConfig::SECONDS_PER_DAY;  // 24 hours max for Monte Carlo
	
	while (sim.isRunning() && sim.getCurrentTime() < maxTime)
	{
		// Check for state transitions BEFORE step
		for (Train* train : trains)
		{
			if (train && train->getCurrentState())
			{
                ITrainState* currentState = train->getCurrentState();
                ITrainState*& prevState = previousStates[train];

                if (currentState != prevState)
                {
                    std::string prevName = prevState ? prevState->getName() : "";
                    std::string currentName = currentState ? currentState->getName() : "";

                    stats.recordStateTransition(train, prevName, currentName);

                    // Track departure (Idle -> Accelerating or other active state)
                    if (prevState == states.idle()
                        && currentState != states.idle())
                    {
                        departureTime[train] = sim.getCurrentTime();
                    }

                    // Track collision avoidance states
                    if (currentState == states.waiting()
                        || currentState == states.emergency())
                    {
                        stats.recordCollisionAvoidance();
                    }

                    prevState = currentState;
                }
			}
		}
		
		sim.step();
		
		// Check if all trains finished
		bool allFinished = true;
		for (Train* train : trains)
		{
			if (train && !train->isFinished())
			{
				allFinished = false;
			}
			else if (train && train->isFinished() && arrivalTime.find(train) == arrivalTime.end())
			{
				// Record arrival time when train first finishes
				arrivalTime[train] = sim.getCurrentTime();
			}
		}
		
		if (allFinished)
		{
			break;
		}
	}
	
	// Finalize metrics
	stats.finalize(sim.getCurrentTime());
	
	// Record network-wide total events generated
	int totalEvents = EventManager::getInstance().getTotalEventsGenerated();
	for (int i = 0; i < totalEvents; ++i)
	{
		stats.recordEventGenerated();
	}
	
	// Set actual travel times
	SimulationMetrics metrics = stats.getMetrics();
	for (Train* train : trains)
	{
		if (train)
		{
			stats.checkTrainDestination(train);
			
			// Calculate actual travel time
			auto depIt = departureTime.find(train);
			auto arrIt = arrivalTime.find(train);
			
			if (depIt != departureTime.end() && arrIt != arrivalTime.end())
			{
				double actualTime = arrIt->second - depIt->second;
				metrics.trainMetrics[train->getName()].actualTravelTime = actualTime;
			}
			
			metrics.trainMetrics[train->getName()].reachedDestination = train->isFinished();
		}
	}
	
	// Cleanup - CRITICAL: Clear SimulationManager references before deleting
	sim.reset();  // Clear train references in singleton FIRST
	
	for (Train* train : trains)
	{
		delete train;
	}
	delete graph;
	
	return metrics;
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
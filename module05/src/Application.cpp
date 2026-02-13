#include "Application.hpp"
#include "io/FileParser.hpp"
#include "io/RailNetworkParser.hpp"
#include "io/TrainConfigParser.hpp"
#include "io/FileOutputWriter.hpp"
#include "io/IOutputWriter.hpp"
#include "io/ConsoleOutputWriter.hpp"
#include "patterns/factories/TrainFactory.hpp"
#include "patterns/strategies/IPathfindingStrategy.hpp"
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "patterns/strategies/AStarStrategy.hpp"
#include "patterns/states/IdleState.hpp"
#include "simulation/SimulationManager.hpp"
#include "analysis/MonteCarloRunner.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include "rendering/SFMLRenderer.hpp"


Application::Application(int argc, char* argv[]) 
	: _cli(argc, argv),
	  _consoleWriter(new ConsoleOutputWriter())
{
}

Application::~Application()
{
	delete _consoleWriter;
}



int Application::run()
{
	if (_cli.shouldShowHelp())
	{
		_cli.printHelp();
		return 0;
	}

	if (!_cli.hasValidArguments())
	{
		_consoleWriter->writeError("Invalid number of arguments");
		_cli.printUsage("railway_sim");
		return 1;
	}
	
	// Validate flag values
	std::string flagError;
	if (!_cli.validateFlags(flagError))
	{
		_consoleWriter->writeError(flagError);
		_consoleWriter->writeError("Use --help for valid options");
		return 1;
	}

	std::string networkFile = _cli.getNetworkFile();
	std::string trainFile = _cli.getTrainFile();

	try
	{
		FileParser::validateFile(networkFile);
		FileParser::validateFile(trainFile);
	}
	catch (const std::exception& e)
	{
		_consoleWriter->writeError(e.what());
		return 1;
	}

	_consoleWriter->writeStartupHeader();
	_consoleWriter->writeConfiguration("Network file", networkFile);
	_consoleWriter->writeConfiguration("Train file", trainFile);
	_consoleWriter->writeConfiguration("Output directory", "output/");
	_consoleWriter->writeConfiguration("Pathfinding", _cli.getPathfinding());
	
	if (_cli.hasRender())
	{
		_consoleWriter->writeConfiguration("Rendering", "enabled (SFML)");
	}
	if (_cli.hasHotReload())
	{
		_consoleWriter->writeConfiguration("Hot-reload", "enabled");
	}
	if (_cli.hasMonteCarloRuns())
	{
		std::string runs = std::to_string(_cli.getMonteCarloRuns()) + " runs";
		_consoleWriter->writeConfiguration("Monte Carlo", runs);
	}

	// Monte Carlo mode - run multiple simulations and exit
	if (_cli.hasMonteCarloRuns())
	{
		try
		{
			// Ensure output directory exists (same pattern as FileOutputWriter)
			#ifdef _WIN32
				_mkdir("output");
			#else
				mkdir("output", 0755);
			#endif
			
			MonteCarloRunner runner(networkFile, trainFile, 
			                        _cli.getSeed(), 
			                        _cli.getMonteCarloRuns(),
			                        _cli.getPathfinding());
			
			runner.runAll();
			runner.writeCSV("output/monte_carlo_results.csv");
			
			return 0;
		}
		catch (const std::exception& e)
		{
			_consoleWriter->writeError(e.what());
			return 1;
		}
	}

	Graph* graph = nullptr;
	std::vector<TrainConfig> trainConfigs;
	std::vector<Train*> trains;
	std::vector<FileOutputWriter*> writers;

	try
	{
		// Parse network
		_consoleWriter->writeProgress("Parsing network file...");
		RailNetworkParser networkParser(networkFile);
		graph = networkParser.parse();

		_consoleWriter->writeGraphDetails(graph->getNodes(), graph->getRails());
		_consoleWriter->writeNetworkSummary(graph->getNodeCount(), graph->getRailCount());

		// Parse train configurations
		_consoleWriter->writeProgress("Parsing train file...");
		TrainConfigParser trainParser(trainFile);
		trainConfigs = trainParser.parse();
		
		std::string trainCount = std::to_string(trainConfigs.size()) + " trains parsed";
		_consoleWriter->writeProgress(trainCount);

		// Create trains and find paths
		_consoleWriter->writeProgress("Creating trains and finding paths...");
		
		// Select pathfinding strategy based on CLI flag
		IPathfindingStrategy* strategy = nullptr;
		DijkstraStrategy dijkstra;
		AStarStrategy astar;
		
		std::string pathfindingAlgo = _cli.getPathfinding();
		if (pathfindingAlgo == "astar")
		{
			strategy = &astar;
			_consoleWriter->writeProgress("Using A* pathfinding");
		}
		else
		{
			strategy = &dijkstra;
			_consoleWriter->writeProgress("Using Dijkstra pathfinding");
		}
		
		static IdleState idleState;

		for (const auto& config : trainConfigs)
		{
			// Create train
			Train* train = TrainFactory::create(config, graph);
			if (!train)
			{
				_consoleWriter->writeError("Failed to create train: " + config.name);
				continue;
			}

			// Find path
			Node* startNode = graph->getNode(config.departureStation);
			Node* endNode = graph->getNode(config.arrivalStation);

			if (!startNode || !endNode)
			{
				_consoleWriter->writeError("Invalid stations for train " + config.name);
				delete train;
				continue;
			}

			auto path = strategy->findPath(graph, startNode, endNode);

			if (path.empty())
			{
				std::string msg = "No path found for train " + config.name + 
				                  " from " + config.departureStation + 
				                  " to " + config.arrivalStation;
				_consoleWriter->writeError(msg);
				delete train;
				continue;
			}

			train->setPath(path);
			_consoleWriter->writePathDebug(train);
			train->setState(&idleState);
			trains.push_back(train);

			_consoleWriter->writeTrainCreated(train->getName(), train->getID(),
			                                   config.departureStation, 
			                                   config.arrivalStation,
			                                   static_cast<int>(path.size()));
		}

		// Create output writers
		_consoleWriter->writeProgress("Creating output files...");
		
		for (Train* train : trains)
		{
			FileOutputWriter* writer = new FileOutputWriter(train);
			writer->open();
			
			// Calculate estimated travel time
			double estimatedTimeMinutes = 0.0;
			for (const PathSegment& segment : train->getPath())
			{
				double segmentTimeHours = segment.rail->getLength() / segment.rail->getSpeedLimit();
				estimatedTimeMinutes += segmentTimeHours * 60.0;
			}
			
			writer->writeHeader(estimatedTimeMinutes);
			writer->writePathInfo();  // Write path information
			writers.push_back(writer);
			
			std::string msg = "Created: output/" + train->getName() + "_" + 
			                  train->getDepartureTime().toString() + ".result" +
			                  " (estimated: " + std::to_string(static_cast<int>(estimatedTimeMinutes)) + " min)";
			_consoleWriter->writeProgress(msg);
		}

		// Initialize simulation
		_consoleWriter->writeProgress("Initializing simulation...");
		SimulationManager& sim = SimulationManager::getInstance();
		sim.reset();
		
		// Connect console output (Dependency Inversion - pass interface)
		sim.setSimulationWriter(_consoleWriter);
		
		// Set seed for event generation
		unsigned int seed;
		if (_cli.hasSeed())
		{
			// Deterministic mode: use specified seed
			seed = _cli.getSeed();
			sim.setEventSeed(seed);
			_consoleWriter->writeConfiguration("Seed", std::to_string(seed) + " (deterministic)");
		}
		else
		{
			// Random mode: use time-based seed and log it for reproducibility
			seed = static_cast<unsigned int>(std::time(nullptr));
			sim.setEventSeed(seed);
			std::string seedMsg = std::to_string(seed) + " (random - use --seed=" + 
			                      std::to_string(seed) + " to reproduce)";
			_consoleWriter->writeConfiguration("Seed", seedMsg);
		}
		
		sim.setNetwork(graph);

		// Register output writers with SimulationManager
		for (size_t i = 0; i < trains.size(); ++i)
		{
			sim.registerOutputWriter(trains[i], writers[i]);
		}

		for (Train* train : trains)
		{
			sim.addTrain(train);
		}

		// Enable round-trip mode if requested
		if (_cli.hasRoundTrip() || _cli.hasRender())
		{
			sim.setRoundTripMode(true);
			_consoleWriter->writeConfiguration("Round-trip mode", "enabled (trains reverse at destination)");
		}

		_consoleWriter->writeSimulationStart();
        for (Train* train : trains)
        {
            _consoleWriter->writeTrainSchedule(train->getName(), train->getDepartureTime());
        }

		// Run simulation
		// If round-trip enabled, run for extended time (or indefinitely for render mode)
		double maxTime = 106400.0;  // Default: ~29 hours
		if (_cli.hasRoundTrip() || _cli.hasRender())
		{
			maxTime = 172800.0;  // 48 hours for round-trip demo
			if (_cli.hasRender())
			{
				SFMLRenderer renderer;
				renderer.run(sim);

				maxTime = 1e9;  // Near-infinite for rendering (user will Ctrl+C)
			}
		}
		sim.run(maxTime);


		// Write final snapshots
		_consoleWriter->writeProgress("Writing final snapshots...");
		for (FileOutputWriter* writer : writers)
		{
			writer->writeSnapshot(sim.getCurrentTime());
			writer->close();
		}

		_consoleWriter->writeSimulationComplete();
		for (const Train* train : trains)
		{
			std::string filename = "output/" + train->getName() + "_" + 
			                       train->getDepartureTime().toString() + ".result";
			_consoleWriter->writeOutputFileListing(filename);
		}

		// Cleanup
		for (FileOutputWriter* writer : writers)
		{
			delete writer;
		}
		
		for (Train* train : trains)
		{
			delete train;
		}

		delete graph;
	}
	catch (const std::exception& e)
	{
		_consoleWriter->writeError(e.what());

		// Cleanup on error
		for (FileOutputWriter* writer : writers)
		{
			delete writer;
		}
		
		for (Train* train : trains)
		{
			delete train;
		}
		
		delete graph;

		return 1;
	}

	return 0;
}
#include "Application.hpp"
#include "io/FileParser.hpp"
#include "io/RailNetworkParser.hpp"
#include "io/TrainConfigParser.hpp"
#include "io/OutputWriter.hpp"
#include "patterns/factories/TrainFactory.hpp"
#include "patterns/strategies/IPathfindingStrategy.hpp"
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "patterns/strategies/AStarStrategy.hpp"
#include "patterns/states/IdleState.hpp"
#include "simulation/SimulationManager.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include <iostream>
#include <ctime>

Application::Application(int argc, char* argv[]) : _cli(argc, argv)
{
}

static void printPath(const Train* train)
{
    std::cout << "\n[DEBUG] PATH FOR TRAIN: " << train->getName() << "\n";

    const auto& path = train->getPath();

    if (path.empty())
    {
        std::cout << "  -> NO PATH FOUND!\n";
        return;
    }

    for (size_t i = 0; i < path.size(); ++i)
    {
        const PathSegment& segment = path[i];
        Rail* rail = segment.rail;

        std::cout << "  Segment " << i
                  << ": "
                  << rail->getNodeA()->getName()
                  << " <-> "
                  << rail->getNodeB()->getName()
                  << " | length="
                  << rail->getLength()
                  << " | speed="
                  << rail->getSpeedLimit()
                  << "\n";
    }

    std::cout << std::endl;
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
		std::cerr << "Error: Invalid number of arguments\n" << std::endl;
		_cli.printUsage("railway_sim");
		return 1;
	}
	
	// Validate flag values
	std::string flagError;
	if (!_cli.validateFlags(flagError))
	{
		std::cerr << "Error: " << flagError << std::endl;
		std::cerr << "Use --help for valid options" << std::endl;
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
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	std::cout << "Railway Simulation Starting..." << std::endl;
	std::cout << "Network file: " << networkFile << std::endl;
	std::cout << "Train file:   " << trainFile << std::endl;
	
	// Log optional flags
	std::cout << "Pathfinding:  " << _cli.getPathfinding() << std::endl;
	if (_cli.hasRender())
	{
		std::cout << "Rendering:    enabled (SFML)" << std::endl;
	}
	if (_cli.hasHotReload())
	{
		std::cout << "Hot-reload:   enabled" << std::endl;
	}
	if (_cli.hasMonteCarloRuns())
	{
		std::cout << "Monte Carlo:  " << _cli.getMonteCarloRuns() << " runs" << std::endl;
	}
	
	std::cout << std::endl;

	Graph* graph = nullptr;
	std::vector<TrainConfig> trainConfigs;
	std::vector<Train*> trains;
	std::vector<OutputWriter*> writers;

	try
	{
		// Parse network
		std::cout << "Parsing network file..." << std::endl;
		RailNetworkParser networkParser(networkFile);
		graph = networkParser.parse();

        std::cout << "\n=== GRAPH CONTENT ===" << std::endl;
        for (const Node* n : graph->getNodes())
        {
            std::cout << "Node: " << n->getName() << " | type=" << n->getTypeString()
              << std::endl;
        }

        for (const Rail* r : graph->getRails())
        {
            std::cout << "Rail: "
                    << r->getNodeA()->getName()
                    << " <-> "
                    << r->getNodeB()->getName()
                    << std::endl;
        }
        std::cout << "=====================\n" << std::endl;

		std::cout << "  Nodes: " << graph->getNodeCount() << std::endl;
		std::cout << "  Rails: " << graph->getRailCount() << std::endl;
		std::cout << std::endl;

		// Parse train configurations
		std::cout << "Parsing train file..." << std::endl;
		TrainConfigParser trainParser(trainFile);
		trainConfigs = trainParser.parse();
		std::cout << "  Trains: " << trainConfigs.size() << std::endl;
		std::cout << std::endl;

		// Create trains and find paths
		std::cout << "Creating trains and finding paths..." << std::endl;
		
		// Select pathfinding strategy based on CLI flag
		IPathfindingStrategy* strategy = nullptr;
		DijkstraStrategy dijkstra;
		AStarStrategy astar;
		
		std::string pathfindingAlgo = _cli.getPathfinding();
		if (pathfindingAlgo == "astar")
		{
			strategy = &astar;
			std::cout << "  Using A* pathfinding" << std::endl;
		}
		else
		{
			strategy = &dijkstra;
			std::cout << "  Using Dijkstra pathfinding" << std::endl;
		}
		
		static IdleState idleState;

		for (const auto& config : trainConfigs)
		{
			// Create train
			Train* train = TrainFactory::create(config, graph);
			if (!train)
			{
				std::cerr << "  Failed to create train: " << config.name << std::endl;
				continue;
			}

			// Find path
			Node* startNode = graph->getNode(config.departureStation);
			Node* endNode = graph->getNode(config.arrivalStation);

			if (!startNode || !endNode)
			{
				std::cerr << "  Invalid stations for train " << config.name << std::endl;
				delete train;
				continue;
			}

			auto path = strategy->findPath(graph, startNode, endNode);

			if (path.empty())
			{
				std::cerr << "  No path found for train " << config.name 
				          << " from " << config.departureStation 
				          << " to " << config.arrivalStation << std::endl;
				delete train;
				continue;
			}

			train->setPath(path);
            printPath(train);
			train->setState(&idleState);
			trains.push_back(train);

			std::cout << "  " << train->getName() 
			          << " (ID: " << train->getID() << "): " 
			          << config.departureStation << " Ã¢â€ â€™ " << config.arrivalStation 
			          << " (" << path.size() << " segments)" << std::endl;
		}


		// graph->printAsciiMap();


		std::cout << std::endl;

		// Create output writers
		std::cout << "Creating output files..." << std::endl;
		
		for (Train* train : trains)
		{
			OutputWriter* writer = new OutputWriter(train);
			writer->open();
			
			// Calculate estimated travel time
			double estimatedTimeMinutes = 0.0;
			for (const PathSegment& segment : train->getPath())
			{
				double segmentTimeHours = segment.rail->getLength() / segment.rail->getSpeedLimit();
				estimatedTimeMinutes += segmentTimeHours * 60.0;
			}
			
			writer->writeHeader(estimatedTimeMinutes);
			writers.push_back(writer);
			
			std::cout << "  Created: " << train->getName() << "_" 
			          << train->getDepartureTime().toString() << ".result"
			          << " (estimated: " << static_cast<int>(estimatedTimeMinutes) << " min)"
			          << std::endl;
		}

		std::cout << std::endl;

		// Initialize simulation
		std::cout << "Initializing simulation..." << std::endl;
		SimulationManager& sim = SimulationManager::getInstance();
		sim.reset();
		sim.setNetwork(graph);
		
		// Set seed for event generation
		unsigned int seed;
		if (_cli.hasSeed())
		{
			// Deterministic mode: use specified seed
			seed = _cli.getSeed();
			sim.setEventSeed(seed);
			std::cout << "  Event seed: " << seed << " (deterministic mode)" << std::endl;
		}
		else
		{
			// Random mode: use time-based seed and log it for reproducibility
			seed = static_cast<unsigned int>(std::time(nullptr));
			sim.setEventSeed(seed);
			std::cout << "  Event seed: " << seed << " (random mode - use --seed=" 
			          << seed << " to reproduce)" << std::endl;
		}

		for (Train* train : trains)
		{
			sim.addTrain(train);
		}

		std::cout << std::endl;
		std::cout << "===== SIMULATION START =====" << std::endl;
		std::cout << std::endl;

        for (Train* train : trains)
        {
            std::cout << "Train "
                    << train->getName()
                    << " scheduled for "
                    << train->getDepartureTime().toString()
                    << std::endl;
        }


		// Run simulation step by step with snapshot writing
		sim.run(106400.0);


		// Write final snapshots
		std::cout << std::endl;
		std::cout << "Writing final snapshots..." << std::endl;
		for (OutputWriter* writer : writers)
		{
			writer->writeSnapshot(sim.getCurrentTime());
			writer->close();
		}

		std::cout << std::endl;
		std::cout << "===== SIMULATION COMPLETE =====" << std::endl;
		std::cout << std::endl;
		
		std::cout << "Output files generated:" << std::endl;
		for (const Train* train : trains)
		{
			std::cout << "  " << train->getName() << "_" 
			          << train->getDepartureTime().toString() << ".result" << std::endl;
		}

		// Cleanup
		for (OutputWriter* writer : writers)
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
		std::cerr << "Error: " << e.what() << std::endl;

		// Cleanup on error
		for (OutputWriter* writer : writers)
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
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
#include "utils/ConsoleColors.hpp"
#include <iostream>
#include <ctime>

Application::Application(int argc, char* argv[]) : _cli(argc, argv)
{
}

static void printPath(const Train* train)
{
    std::cout << "\nPATH FOR TRAIN: " << train->getName() << "\n";

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

	std::cout << "\n" << Color::BOLD_CYAN;
	std::cout << Box::drawHeader("RAILWAY SIMULATION ENGINE", 80);
	std::cout << Color::RESET;
	
	std::cout << Color::CYAN << "Network file: " << Color::RESET << networkFile << std::endl;
	std::cout << Color::CYAN << "Train file:   " << Color::RESET << trainFile << std::endl;
	
	// Log optional flags
	std::cout << Color::CYAN << "Pathfinding:  " << Color::RESET 
	          << Color::BOLD_GREEN << _cli.getPathfinding() << Color::RESET << std::endl;
	if (_cli.hasRender())
	{
		std::cout << Color::CYAN << "Rendering:    " << Color::RESET 
		          << Color::BOLD_GREEN << "enabled (SFML)" << Color::RESET << std::endl;
	}
	if (_cli.hasHotReload())
	{
		std::cout << Color::CYAN << "Hot-reload:   " << Color::RESET 
		          << Color::BOLD_GREEN << "enabled" << Color::RESET << std::endl;
	}
	if (_cli.hasMonteCarloRuns())
	{
		std::cout << Color::CYAN << "Monte Carlo:  " << Color::RESET 
		          << Color::BOLD_GREEN << _cli.getMonteCarloRuns() << " runs" << Color::RESET << std::endl;
	}
	
	std::cout << std::endl;

	Graph* graph = nullptr;
	std::vector<TrainConfig> trainConfigs;
	std::vector<Train*> trains;
	std::vector<OutputWriter*> writers;

	try
	{
		// Parse network
		std::cout << "\n" << Color::BOLD_YELLOW << "► Parsing network file..." << Color::RESET << std::endl;
		RailNetworkParser networkParser(networkFile);
		graph = networkParser.parse();

        std::cout << Color::DIM << "\n=== GRAPH CONTENT ===" << Color::RESET << std::endl;
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
        std::cout << Color::DIM << "=====================\n" << Color::RESET << std::endl;

		std::cout << Color::GREEN << "  ✓ Nodes: " << Color::RESET << graph->getNodeCount() << std::endl;
		std::cout << Color::GREEN << "  ✓ Rails: " << Color::RESET << graph->getRailCount() << std::endl;
		std::cout << std::endl;

		// Parse train configurations
		std::cout << Color::BOLD_YELLOW << "► Parsing train file..." << Color::RESET << std::endl;
		TrainConfigParser trainParser(trainFile);
		trainConfigs = trainParser.parse();
		std::cout << Color::GREEN << "  ✓ Trains: " << Color::RESET << trainConfigs.size() << std::endl;
		std::cout << std::endl;

		// Create trains and find paths
		std::cout << Color::BOLD_YELLOW << "► Creating trains and finding paths..." << Color::RESET << std::endl;
		
		// Select pathfinding strategy based on CLI flag
		IPathfindingStrategy* strategy = nullptr;
		DijkstraStrategy dijkstra;
		AStarStrategy astar;
		
		std::string pathfindingAlgo = _cli.getPathfinding();
		if (pathfindingAlgo == "astar")
		{
			strategy = &astar;
			std::cout << Color::BOLD_MAGENTA << "  Using A* pathfinding" << Color::RESET << std::endl;
		}
		else
		{
			strategy = &dijkstra;
			std::cout << Color::BOLD_MAGENTA << "  Using Dijkstra pathfinding" << Color::RESET << std::endl;
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

			std::cout << Color::GREEN << "  ✓ " << Color::RESET
			          << Color::BOLD_WHITE << train->getName() << Color::RESET
			          << Color::DIM << " (ID: " << train->getID() << ")" << Color::RESET
			          << ": "
			          << Color::CYAN << config.departureStation << Color::RESET
			          << " → "
			          << Color::CYAN << config.arrivalStation << Color::RESET
			          << Color::DIM << " (" << path.size() << " segments)" << Color::RESET
			          << std::endl;
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
			std::cout << Color::YELLOW << "  Event seed: " << Color::RESET 
			          << Color::BOLD_WHITE << seed << Color::RESET 
			          << Color::DIM << " (deterministic mode)" << Color::RESET << std::endl;
		}
		else
		{
			// Random mode: use time-based seed and log it for reproducibility
			seed = static_cast<unsigned int>(std::time(nullptr));
			sim.setEventSeed(seed);
			std::cout << Color::YELLOW << "  Event seed: " << Color::RESET 
			          << Color::BOLD_WHITE << seed << Color::RESET 
			          << Color::DIM << " (random mode - use --seed=" 
			          << seed << " to reproduce)" << Color::RESET << std::endl;
		}

		for (Train* train : trains)
		{
			sim.addTrain(train);
		}

		std::cout << "\n" << Color::BOLD_GREEN;
		std::cout << Box::drawHeader("SIMULATION START", 80);
		std::cout << Color::RESET;
		
		std::cout << Color::BOLD_CYAN << "Train Schedule:" << Color::RESET << std::endl;
        for (Train* train : trains)
        {
            std::cout << Color::DIM << "  • " << Color::RESET
                    << Color::BOLD_WHITE << train->getName() << Color::RESET
                    << Color::DIM << " scheduled for " << Color::RESET
                    << Color::YELLOW << train->getDepartureTime().toString() << Color::RESET
                    << std::endl;
        }
		std::cout << std::endl;


		// Run simulation step by step with snapshot writing
		sim.run(106400.0);


		// Write final snapshots
		std::cout << "\n" << Color::BOLD_YELLOW << "► Writing final snapshots..." << Color::RESET << std::endl;
		for (OutputWriter* writer : writers)
		{
			writer->writeSnapshot(sim.getCurrentTime());
			writer->close();
		}

		std::cout << "\n" << Color::BOLD_GREEN;
		std::cout << Box::drawHeader("SIMULATION COMPLETE", 80);
		std::cout << Color::RESET;
		
		std::cout << Color::BOLD_CYAN << "Output files generated:" << Color::RESET << std::endl;
		for (const Train* train : trains)
		{
			std::cout << Color::GREEN << "  ✓ " << Color::RESET
			          << Color::BOLD_WHITE << train->getName() << "_" 
			          << train->getDepartureTime().toString() << ".result" << Color::RESET << std::endl;
		}
		std::cout << std::endl;

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
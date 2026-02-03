#include "Application.hpp"
#include "io/FileParser.hpp"
#include "io/RailNetworkParser.hpp"
#include "io/TrainConfigParser.hpp"
#include "io/OutputWriter.hpp"
#include "patterns/factories/TrainFactory.hpp"
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "patterns/states/IdleState.hpp"
#include "simulation/SimulationManager.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include <iostream>

Application::Application(int argc, char* argv[]) : _cli(argc, argv)
{
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
		DijkstraStrategy dijkstra;
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

			auto path = dijkstra.findPath(graph, startNode, endNode);

			if (path.empty())
			{
				std::cerr << "  No path found for train " << config.name 
				          << " from " << config.departureStation 
				          << " to " << config.arrivalStation << std::endl;
				delete train;
				continue;
			}

			train->setPath(path);
			train->setState(&idleState);
			trains.push_back(train);

			std::cout << "  " << train->getName() 
			          << " (ID: " << train->getID() << "): " 
			          << config.departureStation << " → " << config.arrivalStation 
			          << " (" << path.size() << " segments)" << std::endl;
		}

		std::cout << std::endl;

		// Create output writers
		std::cout << "Creating output files..." << std::endl;
		
		for (Train* train : trains)
		{
			OutputWriter* writer = new OutputWriter(train);
			writer->open();
			
			// Calculate estimated travel time
			double estimatedTimeMinutes = 0.0;
			for (Rail* rail : train->getPath())
			{
				double segmentTimeHours = rail->getLength() / rail->getSpeedLimit();
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

		for (Train* train : trains)
		{
			sim.addTrain(train);
		}

		std::cout << std::endl;
		std::cout << "===== SIMULATION START =====" << std::endl;
		std::cout << std::endl;

		// Run simulation step by step with snapshot writing
		sim.start();
		
		int lastSnapshotMinute = -1;
		int lastProgressHour = -1;

		while (sim.isRunning() && sim.getCurrentTime() < 86400.0) // 24 hours max
		{
			sim.step();
			
			double currentTime = sim.getCurrentTime();
			int currentMinute = static_cast<int>(currentTime / 60.0);
			int currentHour = currentMinute / 60;
			
			// Write snapshots every 5 minutes
			if (currentMinute % 5 == 0 && currentMinute != lastSnapshotMinute)
			{
				for (OutputWriter* writer : writers)
				{
					writer->writeSnapshot(currentTime);
				}
				lastSnapshotMinute = currentMinute;
			}
			
			// Print progress every hour
			if (currentHour > lastProgressHour && currentHour > 0)
			{
				std::cout << "  Simulation time: " << currentHour << "h" << std::endl;
				lastProgressHour = currentHour;
			}
		}

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
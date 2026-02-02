#include "Application.hpp"
#include "io/FileParser.hpp"
#include "io/RailNetworkParser.hpp"
#include "io/TrainConfigParser.hpp"
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

	try
	{
		std::cout << "Parsing network file..." << std::endl;
		RailNetworkParser networkParser(networkFile);
		graph = networkParser.parse();
		std::cout << "  Nodes: " << graph->getNodeCount() << std::endl;
		std::cout << "  Rails: " << graph->getRailCount() << std::endl;
		std::cout << std::endl;

		std::cout << "Parsing train file..." << std::endl;
		TrainConfigParser trainParser(trainFile);
		trainConfigs = trainParser.parse();
		std::cout << "  Trains: " << trainConfigs.size() << std::endl;
		std::cout << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Parsing error: " << e.what() << std::endl;
		delete graph;
		return 1;
	}

	delete graph;

	std::cout << "Simulation complete!" << std::endl;

	return 0;
}
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <unistd.h>

#include "io/RailNetworkParser.hpp"
#include "io/TrainConfigParser.hpp"
#include "patterns/factories/TrainFactory.hpp"
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "core/Train.hpp"

namespace
{
	std::string writeTempFile(const std::string& prefix, const std::string& contents)
	{
		const auto path = std::filesystem::temp_directory_path()
			/ std::filesystem::path(prefix + "_" + std::to_string(::getpid()) + "_" + std::to_string(std::rand()) + ".txt");
		std::ofstream out(path);
		out << contents;
		out.close();
		return path.string();
	}
}

TEST(ParserToTrainPipelineTest, ParsedInputsCreateRunnableTrainPath)
{
	const std::string networkFile = writeTempFile("network_pipeline_test",
		"Node CityA\n"
		"Node CityB\n"
		"Node CityC\n"
		"Rail CityA CityB 15 160\n"
		"Rail CityB CityC 12 140\n");

	const std::string trainFile = writeTempFile("train_pipeline_test",
		"Express 80 0.005 356 500 CityA CityC 06h30 00h05\n");

	RailNetworkParser networkParser(networkFile);
	Graph* graph = networkParser.parse();
	ASSERT_NE(graph, nullptr);
	ASSERT_EQ(graph->getNodeCount(), 3);
	ASSERT_EQ(graph->getRailCount(), 2);

	TrainConfigParser trainParser(trainFile);
	std::vector<TrainConfig> configs = trainParser.parse();
	ASSERT_EQ(configs.size(), 1);

	Train* train = TrainFactory::create(configs[0], graph);
	ASSERT_NE(train, nullptr);

	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(graph, graph->getNode("CityA"), graph->getNode("CityC"));
	ASSERT_EQ(path.size(), 2);

	train->setPath(path);
	EXPECT_NE(train->getCurrentRail(), nullptr);
	EXPECT_EQ(train->getCurrentNode(), graph->getNode("CityA"));
	EXPECT_EQ(train->getNextNode(), graph->getNode("CityB"));

	delete train;
	delete graph;
	std::filesystem::remove(networkFile);
	std::filesystem::remove(trainFile);
}

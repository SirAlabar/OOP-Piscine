#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <cstdlib>
#include <unistd.h>

#include "io/RailNetworkParser.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"

namespace
{
	std::string writeTempFile(const std::string& contents)
	{
		const auto path = std::filesystem::temp_directory_path()
			/ std::filesystem::path("rail_network_parser_test_" + std::to_string(::getpid()) + "_" + std::to_string(std::rand()) + ".txt");
		std::ofstream out(path);
		out << contents;
		out.close();
		return path.string();
	}
}

TEST(RailNetworkParserTest, ParsesValidNetworkWithCommentsAndJunctionType)
{
	const std::string filepath = writeTempFile(
		"# network file\n"
		"Node CityA\n"
		"Node RailNodeHub # junction by name prefix\n"
		"Node CityB\n"
		"Rail CityA RailNodeHub 12.5 140\n"
		"Rail RailNodeHub CityB 7.5 120\n");

	RailNetworkParser parser(filepath);
	Graph* graph = parser.parse();
	ASSERT_NE(graph, nullptr);

	EXPECT_EQ(graph->getNodeCount(), 3);
	EXPECT_EQ(graph->getRailCount(), 2);
	ASSERT_NE(graph->getNode("RailNodeHub"), nullptr);
	EXPECT_EQ(graph->getNode("RailNodeHub")->getType(), NodeType::JUNCTION);

	delete graph;
	std::filesystem::remove(filepath);
}

TEST(RailNetworkParserTest, ThrowsOnDuplicateNode)
{
	const std::string filepath = writeTempFile(
		"Node CityA\n"
		"Node CityA\n");

	RailNetworkParser parser(filepath);
	EXPECT_THROW(parser.parse(), std::runtime_error);

	std::filesystem::remove(filepath);
}

TEST(RailNetworkParserTest, ThrowsOnUnknownRailNode)
{
	const std::string filepath = writeTempFile(
		"Node CityA\n"
		"Rail CityA CityB 10 100\n");

	RailNetworkParser parser(filepath);
	EXPECT_THROW(parser.parse(), std::runtime_error);

	std::filesystem::remove(filepath);
}

TEST(RailNetworkParserTest, ThrowsOnInvalidRailValues)
{
	const std::string filepath = writeTempFile(
		"Node CityA\n"
		"Node CityB\n"
		"Rail CityA CityB -1 100\n");

	RailNetworkParser parser(filepath);
	EXPECT_THROW(parser.parse(), std::runtime_error);

	std::filesystem::remove(filepath);
}

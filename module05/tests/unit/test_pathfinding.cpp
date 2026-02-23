#include <gtest/gtest.h>
#include "patterns/behavioral/strategies/DijkstraStrategy.hpp"
#include "patterns/behavioral/strategies/PathFinder.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"

class PathfindingTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		nodeA = new Node("CityA");
		nodeB = new Node("CityB");
		nodeC = new Node("CityC");
		nodeD = new Node("CityD");
		
		graph.addNode(nodeA);
		graph.addNode(nodeB);
		graph.addNode(nodeC);
		graph.addNode(nodeD);
	}
	
	void TearDown() override
	{
		// Graph destructor handles cleanup
	}
	
	Graph graph;
	Node* nodeA;
	Node* nodeB;
	Node* nodeC;
	Node* nodeD;
};

TEST_F(PathfindingTest, DirectConnection)
{
	Rail* rail = new Rail(nodeA, nodeB, 50.0, 200.0);
	graph.addRail(rail);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeB);
	
	EXPECT_EQ(path.size(), 1);
	EXPECT_EQ(path[0].rail, rail);
}

TEST_F(PathfindingTest, TwoHopPath)
{
	Rail* rail1 = new Rail(nodeA, nodeB, 30.0, 200.0);
	Rail* rail2 = new Rail(nodeB, nodeC, 20.0, 200.0);
	
	graph.addRail(rail1);
	graph.addRail(rail2);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeC);
	
	EXPECT_EQ(path.size(), 2);
	EXPECT_EQ(path[0].rail, rail1);
	EXPECT_EQ(path[1].rail, rail2);
}

TEST_F(PathfindingTest, MultipleRoutesPicksShortest)
{
	Rail* directRail = new Rail(nodeA, nodeB, 50.0, 200.0);
	Rail* railAC = new Rail(nodeA, nodeC, 20.0, 200.0);
	Rail* railCB = new Rail(nodeC, nodeB, 20.0, 200.0);
	
	graph.addRail(directRail);
	graph.addRail(railAC);
	graph.addRail(railCB);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeB);
	
	EXPECT_EQ(path.size(), 2);
	EXPECT_EQ(path[0].rail, railAC);
	EXPECT_EQ(path[1].rail, railCB);
}

TEST_F(PathfindingTest, ConsidersSpeedLimitInCost)
{
	Rail* slowRail = new Rail(nodeA, nodeB, 50.0, 100.0);
	Rail* railAC = new Rail(nodeA, nodeC, 30.0, 300.0);
	Rail* railCB = new Rail(nodeC, nodeB, 30.0, 300.0);
	
	graph.addRail(slowRail);
	graph.addRail(railAC);
	graph.addRail(railCB);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeB);
	
	EXPECT_EQ(path.size(), 2);
}

TEST_F(PathfindingTest, NoPathReturnsEmptyVector)
{
	Rail* rail = new Rail(nodeA, nodeB, 50.0, 200.0);
	graph.addRail(rail);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeD);
	
	EXPECT_TRUE(path.empty());
}

TEST_F(PathfindingTest, ComplexNetworkFindsOptimalPath)
{
	Rail* railAB = new Rail(nodeA, nodeB, 10.0, 100.0);
	Rail* railAC = new Rail(nodeA, nodeC, 5.0, 100.0);
	Rail* railBD = new Rail(nodeB, nodeD, 3.0, 100.0);
	Rail* railCD = new Rail(nodeC, nodeD, 8.0, 100.0);
	
	graph.addRail(railAB);
	graph.addRail(railAC);
	graph.addRail(railBD);
	graph.addRail(railCD);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeD);
	
	EXPECT_EQ(path.size(), 2);
	
	double totalDistance = 0.0;
	for (const auto& segment : path)
	{
		totalDistance += segment.rail->getLength();
	}
	
	EXPECT_DOUBLE_EQ(totalDistance, 13.0);
}

TEST_F(PathfindingTest, PathFinderUsesConfiguredStrategy)
{
	Rail* rail = new Rail(nodeA, nodeB, 50.0, 200.0);
	graph.addRail(rail);
	
	DijkstraStrategy dijkstra;
	PathFinder finder(&dijkstra);
	
	auto path = finder.findPath(&graph, nodeA, nodeB);
	
	EXPECT_EQ(path.size(), 1);
	EXPECT_EQ(path[0].rail, rail);
}

TEST_F(PathfindingTest, SameStartAndEndReturnsEmptyPath)
{
	Rail* rail = new Rail(nodeA, nodeB, 50.0, 200.0);
	graph.addRail(rail);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeA);
	
	EXPECT_TRUE(path.empty());
}

TEST_F(PathfindingTest, NullNodesReturnEmptyPath)
{
	Rail* rail = new Rail(nodeA, nodeB, 50.0, 200.0);
	graph.addRail(rail);
	
	DijkstraStrategy dijkstra;
	
	auto path1 = dijkstra.findPath(&graph, nullptr, nodeB);
	EXPECT_TRUE(path1.empty());
	
	auto path2 = dijkstra.findPath(&graph, nodeA, nullptr);
	EXPECT_TRUE(path2.empty());
	
	auto path3 = dijkstra.findPath(&graph, nullptr, nullptr);
	EXPECT_TRUE(path3.empty());
}
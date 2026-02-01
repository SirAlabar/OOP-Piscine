#include <gtest/gtest.h>
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "patterns/strategies/PathFinder.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"

class PathfindingTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		// Create nodes
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
		delete nodeA;
		delete nodeB;
		delete nodeC;
		delete nodeD;
	}
	
	Graph graph;
	Node* nodeA;
	Node* nodeB;
	Node* nodeC;
	Node* nodeD;
};

TEST_F(PathfindingTest, DirectConnection)
{
	Rail rail(nodeA, nodeB, 50.0, 200.0);
	graph.addRail(&rail);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeB);
	
	EXPECT_EQ(path.size(), 1);
	EXPECT_EQ(path[0], &rail);
}

TEST_F(PathfindingTest, TwoHopPath)
{
	Rail rail1(nodeA, nodeB, 30.0, 200.0);
	Rail rail2(nodeB, nodeC, 20.0, 200.0);
	
	graph.addRail(&rail1);
	graph.addRail(&rail2);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeC);
	
	EXPECT_EQ(path.size(), 2);
	EXPECT_EQ(path[0], &rail1);
	EXPECT_EQ(path[1], &rail2);
}

TEST_F(PathfindingTest, MultipleRoutesPicksShortest)
{
	// Direct: A->B (50km at 200km/h = 0.25h)
	Rail directRail(nodeA, nodeB, 50.0, 200.0);
	
	// Via C: A->C->B (20km at 200 + 20km at 200 = 0.2h) - FASTER
	Rail railAC(nodeA, nodeC, 20.0, 200.0);
	Rail railCB(nodeC, nodeB, 20.0, 200.0);
	
	graph.addRail(&directRail);
	graph.addRail(&railAC);
	graph.addRail(&railCB);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeB);
	
	// Should pick via C (2 hops, less time)
	EXPECT_EQ(path.size(), 2);
	EXPECT_EQ(path[0], &railAC);
	EXPECT_EQ(path[1], &railCB);
}

TEST_F(PathfindingTest, SpeedLimitAffectsCost)
{
	// Route 1: A->B (100km at 100km/h = 1.0h)
	Rail slowRail(nodeA, nodeB, 100.0, 100.0);
	
	// Route 2: A->C->B (80km at 200 + 80km at 200 = 0.8h) - FASTER
	Rail railAC(nodeA, nodeC, 80.0, 200.0);
	Rail railCB(nodeC, nodeB, 80.0, 200.0);
	
	graph.addRail(&slowRail);
	graph.addRail(&railAC);
	graph.addRail(&railCB);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeB);
	
	// Should pick faster route even though longer distance
	EXPECT_EQ(path.size(), 2);
}

TEST_F(PathfindingTest, NoPathExists)
{
	Rail rail(nodeA, nodeB, 50.0, 200.0);
	graph.addRail(&rail);
	
	// C and D are isolated
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeC);
	
	EXPECT_TRUE(path.empty());
}

TEST_F(PathfindingTest, StartEqualsEnd)
{
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeA);
	
	EXPECT_TRUE(path.empty());
}

TEST_F(PathfindingTest, NullGraph)
{
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(nullptr, nodeA, nodeB);
	
	EXPECT_TRUE(path.empty());
}

TEST_F(PathfindingTest, NullStartNode)
{
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nullptr, nodeB);
	
	EXPECT_TRUE(path.empty());
}

TEST_F(PathfindingTest, NullEndNode)
{
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nullptr);
	
	EXPECT_TRUE(path.empty());
}

TEST_F(PathfindingTest, ComplexNetwork)
{
	// Diamond shape: A->B->D, A->C->D
	Rail railAB(nodeA, nodeB, 30.0, 250.0);
	Rail railAC(nodeA, nodeC, 25.0, 150.0);
	Rail railBD(nodeB, nodeD, 20.0, 250.0);
	Rail railCD(nodeC, nodeD, 30.0, 200.0);
	
	graph.addRail(&railAB);
	graph.addRail(&railAC);
	graph.addRail(&railBD);
	graph.addRail(&railCD);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeD);
	
	EXPECT_EQ(path.size(), 2);
	// Should pick A->B->D (faster total time)
	EXPECT_EQ(path[0], &railAB);
	EXPECT_EQ(path[1], &railBD);
}

TEST_F(PathfindingTest, GetStrategyName)
{
	DijkstraStrategy dijkstra;
	EXPECT_EQ(dijkstra.getName(), "Dijkstra");
}

TEST_F(PathfindingTest, PathFinderDefaultConstructor)
{
	PathFinder pf;
	EXPECT_EQ(pf.getStrategy(), nullptr);
}

TEST_F(PathfindingTest, PathFinderWithStrategy)
{
	DijkstraStrategy dijkstra;
	PathFinder pf(&dijkstra);
	
	EXPECT_EQ(pf.getStrategy(), &dijkstra);
}

TEST_F(PathfindingTest, PathFinderSetStrategy)
{
	DijkstraStrategy dijkstra;
	PathFinder pf;
	
	pf.setStrategy(&dijkstra);
	EXPECT_EQ(pf.getStrategy(), &dijkstra);
}

TEST_F(PathfindingTest, PathFinderFindPath)
{
	Rail rail(nodeA, nodeB, 50.0, 200.0);
	graph.addRail(&rail);
	
	DijkstraStrategy dijkstra;
	PathFinder pf(&dijkstra);
	
	auto path = pf.findPath(&graph, nodeA, nodeB);
	
	EXPECT_EQ(path.size(), 1);
	EXPECT_EQ(path[0], &rail);
}

TEST_F(PathfindingTest, PathFinderNoStrategy)
{
	PathFinder pf;
	auto path = pf.findPath(&graph, nodeA, nodeB);
	
	EXPECT_TRUE(path.empty());
}

TEST_F(PathfindingTest, PathFinderSwitchStrategy)
{
	Rail rail(nodeA, nodeB, 50.0, 200.0);
	graph.addRail(&rail);
	
	DijkstraStrategy dijkstra;
	PathFinder pf;
	
	// No strategy - no path
	auto path1 = pf.findPath(&graph, nodeA, nodeB);
	EXPECT_TRUE(path1.empty());
	
	// Set strategy - finds path
	pf.setStrategy(&dijkstra);
	auto path2 = pf.findPath(&graph, nodeA, nodeB);
	EXPECT_EQ(path2.size(), 1);
}
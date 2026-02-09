#include <gtest/gtest.h>
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"

TEST(GraphTest, DefaultConstructor)
{
	Graph g;
	EXPECT_EQ(g.getNodeCount(), 0);
	EXPECT_EQ(g.getRailCount(), 0);
}

TEST(GraphTest, AddSingleNode)
{
	Graph g;
	Node* a = new Node("CityA");

	g.addNode(a);

	EXPECT_EQ(g.getNodeCount(), 1);
	EXPECT_TRUE(g.hasNode("CityA"));
	EXPECT_EQ(g.getNode("CityA"), a);
	// Graph destructor deletes 'a'
}

TEST(GraphTest, AddDuplicateNodeByName)
{
	Graph g;
	Node* a = new Node("CityA");
	Node* b = new Node("CityA");

	g.addNode(a);
	g.addNode(b);

	EXPECT_EQ(g.getNodeCount(), 1);
	
	delete b;  // Not added to graph, must delete manually
}

TEST(GraphTest, AddMultipleNodes)
{
	Graph g;
	Node* a = new Node("CityA");
	Node* b = new Node("CityB");
	Node* c = new Node("CityC");

	g.addNode(a);
	g.addNode(b);
	g.addNode(c);

	EXPECT_EQ(g.getNodeCount(), 3);
	EXPECT_TRUE(g.hasNode("CityA"));
	EXPECT_TRUE(g.hasNode("CityB"));
	EXPECT_TRUE(g.hasNode("CityC"));
}

TEST(GraphTest, GetNodeReturnsNullForNonExistent)
{
	Graph g;
	EXPECT_EQ(g.getNode("NonExistent"), nullptr);
}

TEST(GraphTest, AddSingleRail)
{
	Graph g;
	Node* a = new Node("CityA");
	Node* b = new Node("CityB");
	Rail* rail = new Rail(a, b, 50.0, 200.0);

	g.addNode(a);
	g.addNode(b);
	g.addRail(rail);

	EXPECT_EQ(g.getRailCount(), 1);
}

TEST(GraphTest, AddMultipleRails)
{
	Graph g;
	Node* a = new Node("CityA");
	Node* b = new Node("CityB");
	Node* c = new Node("CityC");

	g.addNode(a);
	g.addNode(b);
	g.addNode(c);

	Rail* r1 = new Rail(a, b, 50.0, 200.0);
	Rail* r2 = new Rail(b, c, 30.0, 150.0);

	g.addRail(r1);
	g.addRail(r2);

	EXPECT_EQ(g.getRailCount(), 2);
}

TEST(GraphTest, RailRequiresBothNodesInGraph)
{
	Graph g;
	Node* a = new Node("CityA");
	Node* b = new Node("CityB");

	g.addNode(a);

	Rail* rail = new Rail(a, b, 50.0, 200.0);
	g.addRail(rail);

	EXPECT_EQ(g.getRailCount(), 0);
	
	delete b;
	delete rail;
}

TEST(GraphTest, GetRailsFromNodeReturnsEmpty)
{
	Graph g;
	Node* a = new Node("CityA");

	g.addNode(a);

	auto rails = g.getRailsFromNode(a);
	EXPECT_TRUE(rails.empty());
}

TEST(GraphTest, GetRailsFromNodeReturnsSingleRail)
{
	Graph g;
	Node* a = new Node("CityA");
	Node* b = new Node("CityB");
	Rail* rail = new Rail(a, b, 50.0, 200.0);

	g.addNode(a);
	g.addNode(b);
	g.addRail(rail);

	auto rails = g.getRailsFromNode(a);
	EXPECT_EQ(rails.size(), 1);
	EXPECT_EQ(rails[0], rail);
}

TEST(GraphTest, GetRailsFromNodeReturnsMultipleRails)
{
	Graph g;
	Node* a = new Node("CityA");
	Node* b = new Node("CityB");
	Node* c = new Node("CityC");

	g.addNode(a);
	g.addNode(b);
	g.addNode(c);

	Rail* r1 = new Rail(a, b, 50.0, 200.0);
	Rail* r2 = new Rail(a, c, 30.0, 150.0);

	g.addRail(r1);
	g.addRail(r2);

	auto rails = g.getRailsFromNode(a);
	EXPECT_EQ(rails.size(), 2);
}

TEST(GraphTest, GetNeighborsReturnsEmpty)
{
	Graph g;
	Node* a = new Node("CityA");

	g.addNode(a);

	auto neighbors = g.getNeighbors(a);
	EXPECT_TRUE(neighbors.empty());
}

TEST(GraphTest, GetNeighborsReturnsSingleNeighbor)
{
	Graph g;
	Node* a = new Node("CityA");
	Node* b = new Node("CityB");
	Rail* rail = new Rail(a, b, 50.0, 200.0);

	g.addNode(a);
	g.addNode(b);
	g.addRail(rail);

	auto neighbors = g.getNeighbors(a);
	EXPECT_EQ(neighbors.size(), 1);
	EXPECT_EQ(neighbors[0], b);
}

TEST(GraphTest, BidirectionalRailConnectsBothNodes)
{
	Graph g;
	Node* a = new Node("CityA");
	Node* b = new Node("CityB");
	Rail* rail = new Rail(a, b, 50.0, 200.0);

	g.addNode(a);
	g.addNode(b);
	g.addRail(rail);

	auto neighborsA = g.getNeighbors(a);
	auto neighborsB = g.getNeighbors(b);

	EXPECT_EQ(neighborsA.size(), 1);
	EXPECT_EQ(neighborsB.size(), 1);
	EXPECT_EQ(neighborsA[0], b);
	EXPECT_EQ(neighborsB[0], a);
}

TEST(GraphTest, IsValidReturnsTrueForEmptyGraph)
{
	Graph g;
	EXPECT_TRUE(g.isValid());
}

TEST(GraphTest, IsValidReturnsTrueForValidGraph)
{
	Graph g;
	Node* a = new Node("CityA");
	Node* b = new Node("CityB");
	Rail* rail = new Rail(a, b, 50.0, 200.0);

	g.addNode(a);
	g.addNode(b);
	g.addRail(rail);

	EXPECT_TRUE(g.isValid());
}
#include <gtest/gtest.h>
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"

TEST(GraphTest, DefaultConstructor)
{
	Graph g;
	EXPECT_EQ(g.getNodeCount(), 0);
	EXPECT_EQ(g.getRailCount(), 0);
	EXPECT_TRUE(g.isValid());
}

TEST(GraphTest, AddSingleNode)
{
	Graph g;
	Node a("CityA");

	g.addNode(&a);

	EXPECT_EQ(g.getNodeCount(), 1);
	EXPECT_TRUE(g.hasNode("CityA"));
	EXPECT_EQ(g.getNode("CityA"), &a);
}

TEST(GraphTest, AddDuplicateNodeByName)
{
	Graph g;
	Node a("CityA");
	Node b("CityA");

	g.addNode(&a);
	g.addNode(&b);

	EXPECT_EQ(g.getNodeCount(), 1);
	EXPECT_EQ(g.getNode("CityA"), &a);
}

TEST(GraphTest, GetNodeNonExistent)
{
	Graph g;
	Node a("CityA");

	g.addNode(&a);

	EXPECT_EQ(g.getNode("CityB"), nullptr);
}

TEST(GraphTest, AddRailBetweenTwoNodes)
{
	Graph g;
	Node a("CityA");
	Node b("CityB");

	g.addNode(&a);
	g.addNode(&b);

	Rail r(&a, &b, 10.0, 100.0);
	g.addRail(&r);

	EXPECT_EQ(g.getRailCount(), 1);
}

TEST(GraphTest, AddRailFailsIfNodeMissing)
{
	Graph g;
	Node a("CityA");
	Node b("CityB");

	g.addNode(&a);

	Rail r(&a, &b, 10.0, 100.0);
	g.addRail(&r);

	EXPECT_EQ(g.getRailCount(), 0);
}

TEST(GraphTest, GetRailsFromNode)
{
	Graph g;
	Node a("CityA");
	Node b("CityB");

	g.addNode(&a);
	g.addNode(&b);

	Rail r(&a, &b, 10.0, 100.0);
	g.addRail(&r);

	auto railsFromA = g.getRailsFromNode(&a);
	EXPECT_EQ(railsFromA.size(), 1);
	EXPECT_EQ(railsFromA[0], &r);
}

TEST(GraphTest, GetNeighbors)
{
	Graph g;
	Node a("CityA");
	Node b("CityB");
	Node c("CityC");

	g.addNode(&a);
	g.addNode(&b);
	g.addNode(&c);

	Rail r1(&a, &b, 10.0, 100.0);
	Rail r2(&a, &c, 15.0, 80.0);

	g.addRail(&r1);
	g.addRail(&r2);

	auto neighbors = g.getNeighbors(&a);

	EXPECT_EQ(neighbors.size(), 2);
	EXPECT_TRUE(neighbors[0] == &b || neighbors[1] == &b);
	EXPECT_TRUE(neighbors[0] == &c || neighbors[1] == &c);
}

TEST(GraphTest, ValidationFailsWithInvalidNode)
{
	Graph g;
	Node invalid("");

	g.addNode(&invalid);

	EXPECT_FALSE(g.isValid());
}

TEST(GraphTest, InvalidRailIsIgnored)
{
	Graph g;
	Node a("CityA");
	Node b("CityB");

	g.addNode(&a);
	g.addNode(&b);

	Rail invalidRail(&a, &b, -10.0, 100.0);
	g.addRail(&invalidRail);

	EXPECT_EQ(g.getRailCount(), 0);
	EXPECT_TRUE(g.isValid());
}


TEST(GraphTest, ClearGraph)
{
	Graph g;
	Node a("CityA");
	Node b("CityB");

	g.addNode(&a);
	g.addNode(&b);

	Rail r(&a, &b, 10.0, 100.0);
	g.addRail(&r);

	g.clear();

	EXPECT_EQ(g.getNodeCount(), 0);
	EXPECT_EQ(g.getRailCount(), 0);
}

TEST(GraphTest, AddNullNodeIsIgnored)
{
	Graph g;

	g.addNode(nullptr);

	EXPECT_EQ(g.getNodeCount(), 0);
	EXPECT_TRUE(g.isValid());
}

TEST(GraphTest, AddNullRailIsIgnored)
{
	Graph g;
	Node a("CityA");
	Node b("CityB");

	g.addNode(&a);
	g.addNode(&b);

	g.addRail(nullptr);

	EXPECT_EQ(g.getRailCount(), 0);
	EXPECT_TRUE(g.isValid());
}

TEST(GraphTest, GetRailsFromIsolatedNode)
{
	Graph g;
	Node a("CityA");

	g.addNode(&a);

	auto rails = g.getRailsFromNode(&a);

	EXPECT_TRUE(rails.empty());
}

TEST(GraphTest, GetNeighborsFromIsolatedNode)
{
	Graph g;
	Node a("CityA");

	g.addNode(&a);

	auto neighbors = g.getNeighbors(&a);

	EXPECT_TRUE(neighbors.empty());
}

TEST(GraphTest, GetRailsFromUnknownNode)
{
	Graph g;
	Node a("CityA");
	Node external("CityX");

	g.addNode(&a);

	auto rails = g.getRailsFromNode(&external);

	EXPECT_TRUE(rails.empty());
}

TEST(GraphTest, ClearRemovesAdjacency)
{
	Graph g;
	Node a("CityA");
	Node b("CityB");

	g.addNode(&a);
	g.addNode(&b);

	Rail r(&a, &b, 10.0, 100.0);
	g.addRail(&r);

	g.clear();

	EXPECT_TRUE(g.getRailsFromNode(&a).empty());
	EXPECT_TRUE(g.getRailsFromNode(&b).empty());
	EXPECT_TRUE(g.isValid());
}





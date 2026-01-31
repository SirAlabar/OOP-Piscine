#include <gtest/gtest.h>
#include "core/Node.hpp"

TEST(NodeTest, DefaultConstructor)
{
	Node n;
	EXPECT_EQ(n.getName(), "");
	EXPECT_EQ(n.getType(), CITY);
	EXPECT_FALSE(n.isValid());
}

TEST(NodeTest, ParameterizedConstructor)
{
	Node n("CityA", CITY);
	EXPECT_EQ(n.getName(), "CityA");
	EXPECT_EQ(n.getType(), CITY);
	EXPECT_TRUE(n.isValid());
}

TEST(NodeTest, ConstructorDefaultsToCity)
{
	Node n("RailNodeA");
	EXPECT_EQ(n.getType(), CITY);
}

TEST(NodeTest, JunctionTypeWorks)
{
	Node n("RailNodeB", JUNCTION);
	EXPECT_EQ(n.getType(), JUNCTION);
	EXPECT_TRUE(n.isValid());
}

TEST(NodeTest, CopyConstructor)
{
	Node original("CityB", JUNCTION);
	Node copy(original);
	EXPECT_EQ(copy.getName(), "CityB");
	EXPECT_EQ(copy.getType(), JUNCTION);
}

TEST(NodeTest, AssignmentOperator)
{
	Node n1("CityC", CITY);
	Node n2("CityD", JUNCTION);
	n2 = n1;
	EXPECT_EQ(n2.getName(), "CityC");
	EXPECT_EQ(n2.getType(), CITY);
}

TEST(NodeTest, SelfAssignment)
{
	Node n("CityE", CITY);
	n = n;
	EXPECT_EQ(n.getName(), "CityE");
	EXPECT_EQ(n.getType(), CITY);
}

TEST(NodeTest, IsValidEmptyName)
{
	Node n("", CITY);
	EXPECT_FALSE(n.isValid());
}

TEST(NodeTest, IsValidWithName)
{
	Node n("ValidName", JUNCTION);
	EXPECT_TRUE(n.isValid());
}

TEST(NodeTest, GetTypeStringCity)
{
	Node n("CityF", CITY);
	EXPECT_EQ(n.getTypeString(), "City");
}

TEST(NodeTest, GetTypeStringJunction)
{
	Node n("RailNodeC", JUNCTION);
	EXPECT_EQ(n.getTypeString(), "Junction");
}
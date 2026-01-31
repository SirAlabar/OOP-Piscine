#include <gtest/gtest.h>
#include "core/Node.hpp"

TEST(NodeTest, DefaultConstructor)
{
	Node n;
	EXPECT_EQ(n.getName(), "");
	EXPECT_EQ(n.getType(), NodeType::CITY);
	EXPECT_FALSE(n.isValid());
}

TEST(NodeTest, ParameterizedConstructor)
{
	Node n("CityA", NodeType::CITY);
	EXPECT_EQ(n.getName(), "CityA");
	EXPECT_EQ(n.getType(), NodeType::CITY);
	EXPECT_TRUE(n.isValid());
}

TEST(NodeTest, ConstructorDefaultsToCity)
{
	Node n("RailNodeA");
	EXPECT_EQ(n.getType(), NodeType::CITY);
}

TEST(NodeTest, JunctionTypeWorks)
{
	Node n("RailNodeB", NodeType::JUNCTION);
	EXPECT_EQ(n.getType(), NodeType::JUNCTION);
	EXPECT_TRUE(n.isValid());
}

TEST(NodeTest, CopyConstructor)
{
	Node original("CityB", NodeType::JUNCTION);
	Node copy(original);
	EXPECT_EQ(copy.getName(), "CityB");
	EXPECT_EQ(copy.getType(), NodeType::JUNCTION);
}

TEST(NodeTest, AssignmentOperator)
{
	Node n1("CityC", NodeType::CITY);
	Node n2("CityD", NodeType::JUNCTION);
	n2 = n1;
	EXPECT_EQ(n2.getName(), "CityC");
	EXPECT_EQ(n2.getType(), NodeType::CITY);
}

TEST(NodeTest, SelfAssignment)
{
	Node n("CityE", NodeType::CITY);
	n = n;
	EXPECT_EQ(n.getName(), "CityE");
	EXPECT_EQ(n.getType(), NodeType::CITY);
}

TEST(NodeTest, IsValidEmptyName)
{
	Node n("", NodeType::CITY);
	EXPECT_FALSE(n.isValid());
}

TEST(NodeTest, IsValidWithName)
{
	Node n("ValidName", NodeType::JUNCTION);
	EXPECT_TRUE(n.isValid());
}

TEST(NodeTest, GetTypeStringCity)
{
	Node n("CityF", NodeType::CITY);
	EXPECT_EQ(n.getTypeString(), "City");
}

TEST(NodeTest, GetTypeStringJunction)
{
	Node n("RailNodeC", NodeType::JUNCTION);
	EXPECT_EQ(n.getTypeString(), "Junction");
}

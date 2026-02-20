#include <gtest/gtest.h>
#include "core/Node.hpp"
#include "core/Rail.hpp"

TEST(RailTest, DefaultConstructor)
{
    Rail r;
    EXPECT_EQ(r.getNodeA(), nullptr);
    EXPECT_EQ(r.getNodeB(), nullptr);
    EXPECT_EQ(r.getLength(), 0.0);
    EXPECT_EQ(r.getSpeedLimit(), 0.0);
    EXPECT_FALSE(r.isValid());
}

TEST(RailTest, ParameterizedConstructor)
{
    Node a("CityA");
    Node b("CityB");
    Rail r(&a, &b, 10.0, 100.0);
    
    EXPECT_EQ(r.getNodeA(), &a);
    EXPECT_EQ(r.getNodeB(), &b);
    EXPECT_EQ(r.getLength(), 10.0);
    EXPECT_EQ(r.getSpeedLimit(), 100.0);
    EXPECT_TRUE(r.isValid());
}

TEST(RailTest, GetOtherNodeFromA)
{
    Node a("CityA");
    Node b("CityB");
    Rail r(&a, &b, 10.0, 100.0);
    
    EXPECT_EQ(r.getOtherNode(&a), &b);
}

TEST(RailTest, GetOtherNodeFromB)
{
    Node a("CityA");
    Node b("CityB");
    Rail r(&a, &b, 10.0, 100.0);
    
    EXPECT_EQ(r.getOtherNode(&b), &a);
}

TEST(RailTest, GetOtherNodeInvalidNode)
{
    Node a("CityA");
    Node b("CityB");
    Node c("CityC");
    Rail r(&a, &b, 10.0, 100.0);
    
    EXPECT_EQ(r.getOtherNode(&c), nullptr);
    EXPECT_EQ(r.getOtherNode(nullptr), nullptr);
}

TEST(RailTest, CopyConstructor)
{
    Node a("CityA");
    Node b("CityB");
    Rail original(&a, &b, 15.0, 120.0);
    Rail copy(original);
    
    EXPECT_EQ(copy.getNodeA(), &a);
    EXPECT_EQ(copy.getNodeB(), &b);
    EXPECT_EQ(copy.getLength(), 15.0);
    EXPECT_EQ(copy.getSpeedLimit(), 120.0);
}

TEST(RailTest, AssignmentOperator)
{
    Node a("CityA");
    Node b("CityB");
    Node c("CityC");
    Node d("CityD");
    
    Rail r1(&a, &b, 10.0, 100.0);
    Rail r2(&c, &d, 20.0, 200.0);
    
    r2 = r1;
    EXPECT_EQ(r2.getNodeA(), &a);
    EXPECT_EQ(r2.getNodeB(), &b);
    EXPECT_EQ(r2.getLength(), 10.0);
    EXPECT_EQ(r2.getSpeedLimit(), 100.0);
}

TEST(RailTest, IsValidNullNodeA)
{
    Node b("CityB");
    Rail r(nullptr, &b, 10.0, 100.0);
    
    EXPECT_FALSE(r.isValid());
}

TEST(RailTest, IsValidNullNodeB)
{
    Node a("CityA");
    Rail r(&a, nullptr, 10.0, 100.0);
    
    EXPECT_FALSE(r.isValid());
}

TEST(RailTest, IsValidNegativeLength)
{
    Node a("CityA");
    Node b("CityB");
    Rail r(&a, &b, -10.0, 100.0);
    
    EXPECT_FALSE(r.isValid());
}

TEST(RailTest, IsValidZeroLength)
{
    Node a("CityA");
    Node b("CityB");
    Rail r(&a, &b, 0.0, 100.0);
    
    EXPECT_FALSE(r.isValid());
}

TEST(RailTest, IsValidNegativeSpeedLimit)
{
    Node a("CityA");
    Node b("CityB");
    Rail r(&a, &b, 10.0, -100.0);
    
    EXPECT_FALSE(r.isValid());
}

TEST(RailTest, IsValidZeroSpeedLimit)
{
    Node a("CityA");
    Node b("CityB");
    Rail r(&a, &b, 10.0, 0.0);
    
    EXPECT_FALSE(r.isValid());
}
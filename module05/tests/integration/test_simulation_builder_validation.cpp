#include <gtest/gtest.h>

#include <vector>

#include "simulation/SimulationBuilder.hpp"
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "utils/Time.hpp"

class SimulationBuilderValidationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        cityA = new Node("CityA");
        cityB = new Node("CityB");
        cityC = new Node("CityC");

        graph.addNode(cityA);
        graph.addNode(cityB);
        graph.addNode(cityC);

        graph.addRail(new Rail(cityA, cityB, 10.0, 180.0));
    }

    Graph graph;
    Node* cityA;
    Node* cityB;
    Node* cityC;
};

TEST_F(SimulationBuilderValidationTest, ClassifiesRoutableAndNoPathConfigs)
{
    DijkstraStrategy strategy;

    TrainConfig routable = {
        "Routable", 80.0, 0.005, 356.0, 500.0,
        "CityA", "CityB", Time("14h00"), Time("00h05")};

    TrainConfig noPath = {
        "NoPath", 70.0, 0.004, 300.0, 450.0,
        "CityA", "CityC", Time("14h05"), Time("00h04")};

    auto results = SimulationBuilder::validateTrainConfigs({routable, noPath}, &graph, &strategy);

    ASSERT_EQ(results.size(), 2U);
    EXPECT_EQ(results[0].status, TrainValidationResult::Status::Routable);
    EXPECT_FALSE(results[0].path.empty());

    EXPECT_EQ(results[1].status, TrainValidationResult::Status::NoPath);
    EXPECT_TRUE(results[1].path.empty());
    EXPECT_NE(results[1].error.find("No path from CityA to CityC"), std::string::npos);
}

TEST_F(SimulationBuilderValidationTest, ClassifiesInvalidConfigBeforePathfinding)
{
    DijkstraStrategy strategy;

    TrainConfig invalid = {
        "Invalid", -10.0, 0.005, 356.0, 500.0,
        "CityA", "CityB", Time("14h00"), Time("00h05")};

    auto results = SimulationBuilder::validateTrainConfigs({invalid}, &graph, &strategy);

    ASSERT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0].status, TrainValidationResult::Status::InvalidConfig);
    EXPECT_TRUE(results[0].path.empty());
    EXPECT_FALSE(results[0].error.empty());
}

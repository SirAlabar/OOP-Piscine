#include <gtest/gtest.h>
#include "patterns/states/IdleState.hpp"
#include "patterns/states/AcceleratingState.hpp"
#include "patterns/states/StoppedState.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "patterns/mediator/TrafficController.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/CollisionAvoidance.hpp"
#include "utils/Time.hpp"

class TrainStateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Train::resetIDCounter();
        
        nodeA = new Node("CityA");
        nodeB = new Node("CityB");
        
        graph.addNode(nodeA);
        graph.addNode(nodeB);
        
        rail = new Rail(nodeA, nodeB, 50.0, 250.0);
        graph.addRail(rail);
        
        train = new Train("TestTrain", 80.0, 0.005, 356.0, 500.0,
                          "CityA", "CityB",
                          Time("00h00"), Time("00h05"));
        
        train->setPath({{rail, nodeA, nodeB}});
        
        trains.push_back(train);
        
        collisionSystem = new CollisionAvoidance();
        trafficController = new TrafficController(&graph, collisionSystem, &trains);
        context = new SimulationContext(&graph, collisionSystem, &trains, trafficController);
    }
    
    void TearDown() override
    {
        delete train;
        delete nodeA;
        delete nodeB;
        delete rail;
        delete context;
        delete trafficController;
        delete collisionSystem;
    }
    
    Graph graph;
    Node *nodeA, *nodeB;
    Rail *rail;
    Train *train;
    std::vector<Train*> trains;
    CollisionAvoidance *collisionSystem;
    TrafficController *trafficController;
    SimulationContext *context;
};

TEST_F(TrainStateTest, IdleStateKeepsVelocityZero)
{
    IdleState idleState;
    
    train->setVelocity(0.0);
    idleState.update(train, 1.0);
    
    EXPECT_EQ(train->getVelocity(), 0.0);
}

TEST_F(TrainStateTest, AcceleratingStateIncreasesVelocity)
{
    AcceleratingState accelState;
    
    train->setVelocity(0.0);
    train->setPath({ {rail, nodeA, nodeB} });
    
    accelState.update(train, 1.0);
    
    EXPECT_GT(train->getVelocity(), 0.0);
}


TEST_F(TrainStateTest, AcceleratingStateRespectsSpeedLimit)
{
    AcceleratingState accelState;
    
    train->setVelocity(0.0);
    train->setPath({ {rail, nodeA, nodeB} });
	train->advanceToNextRail();

    
    for (int i = 0; i < 100; i++)
    {
        accelState.update(train, 1.0);
    }
    
    double speedLimitMs = 250.0 / 3.6;
    EXPECT_LE(train->getVelocity(), speedLimitMs);
}

TEST_F(TrainStateTest, StoppedStateKeepsVelocityZero)
{
    StoppedState stoppedState;
    
    train->setVelocity(10.0);
    stoppedState.update(train, 1.0);
    
    EXPECT_EQ(train->getVelocity(), 0.0);
}

TEST_F(TrainStateTest, StoppedStateExternalDurationManagement)
{
    context->setStopDuration(train, 10.0);
    
    EXPECT_EQ(context->getStopDuration(train), 10.0);
    
    bool expired = context->decrementStopDuration(train, 1.0);
    EXPECT_FALSE(expired);
    EXPECT_EQ(context->getStopDuration(train), 9.0);
    
    context->decrementStopDuration(train, 3.0);
    EXPECT_EQ(context->getStopDuration(train), 6.0);
}

TEST_F(TrainStateTest, StoppedStateDurationReachesZero)
{
    context->setStopDuration(train, 5.0);
    
    context->decrementStopDuration(train, 3.0);
    EXPECT_EQ(context->getStopDuration(train), 2.0);
    
    bool expired = context->decrementStopDuration(train, 2.0);
    EXPECT_TRUE(expired);
    EXPECT_EQ(context->getStopDuration(train), 0.0);
}

TEST_F(TrainStateTest, StoppedStateDurationDoesNotGoNegative)
{
    context->setStopDuration(train, 2.0);
    
    context->decrementStopDuration(train, 5.0);
    
    EXPECT_EQ(context->getStopDuration(train), 0.0);
}

TEST_F(TrainStateTest, StateTransitionStoppedWithDuration)
{
    StoppedState stoppedState;
    
    train->setVelocity(50.0);
    train->setPath({ {rail, nodeA, nodeB} });
	train->advanceToNextRail();

    
    context->setStopDuration(train, 5.0);
    
    stoppedState.update(train, 1.0);
    
    EXPECT_EQ(train->getVelocity(), 0.0);
    
    context->decrementStopDuration(train, 1.0);
    EXPECT_EQ(context->getStopDuration(train), 4.0);
}

TEST_F(TrainStateTest, TrafficControllerGrantsAccessToEmptyRail)
{
    train->setPath({ {rail, nodeA, nodeB} });
	train->advanceToNextRail();

    
    TrafficController::AccessDecision decision = 
        trafficController->requestRailAccess(train, rail);
    
    EXPECT_EQ(decision, TrafficController::GRANT);
}

TEST_F(TrainStateTest, MultipleTrainsOnRailWithSafeGap)
{
    Train train2("Train2", 80.0, 0.005, 356.0, 500.0,
                 "CityA", "CityB", Time("00h05"), Time("00h05"));
    train2.setPath({{rail, nodeA, nodeB}});
    trains.push_back(&train2);
    
    // Train 1 far ahead
    train->setPath({ {rail, nodeA, nodeB} });
	train->advanceToNextRail();

    train->setPosition(40000.0);  // 40km into 50km rail
    rail->addTrain(train);
    
    // Train 2 requests access - should be granted (safe distance)
    train->setPath({ {rail, nodeA, nodeB} });
	train->advanceToNextRail();

    train2.setPosition(0.0);
    
    TrafficController::AccessDecision decision = 
        trafficController->requestRailAccess(&train2, rail);
    
    EXPECT_EQ(decision, TrafficController::GRANT);
}
#include <gtest/gtest.h>
#include "patterns/states/IdleState.hpp"
#include "patterns/states/AcceleratingState.hpp"
#include "patterns/states/StoppedState.hpp"
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
		
		collisionSystem = new CollisionAvoidance();
		std::vector<Train*> trains;
		context = new SimulationContext(&graph, collisionSystem, &trains);
	}
	
	void TearDown() override
	{
		delete train;
		delete nodeA;
		delete nodeB;
		delete rail;
		delete context;
		delete collisionSystem;
	}
	
	Graph graph;
	Node *nodeA, *nodeB;
	Rail *rail;
	Train *train;
	CollisionAvoidance *collisionSystem;
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
	accelState.update(train, 1.0);
	
	EXPECT_GT(train->getVelocity(), 0.0);
}

TEST_F(TrainStateTest, AcceleratingStateRespectsSpeedLimit)
{
	AcceleratingState accelState;
	
	train->setVelocity(0.0);
	
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

TEST_F(TrainStateTest, StateTransitionAcceleratingToStopped)
{
	StoppedState stoppedState;
	
	train->setVelocity(50.0);
	
	context->setStopDuration(train, 5.0);
	
	stoppedState.update(train, 1.0);
	
	EXPECT_EQ(train->getVelocity(), 0.0);
	
	context->decrementStopDuration(train, 1.0);
	EXPECT_EQ(context->getStopDuration(train), 4.0);
}
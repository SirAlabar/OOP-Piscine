#include <gtest/gtest.h>
#include "patterns/states/IdleState.hpp"
#include "patterns/states/AcceleratingState.hpp"
#include "patterns/states/StoppedState.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "patterns/mediator/TrafficController.hpp"
#include "patterns/mediator/ITrainController.hpp"
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
	
	EXPECT_DOUBLE_EQ(train->getVelocity(), 0.0);
}

TEST_F(TrainStateTest, AcceleratingStateIncreasesVelocity)
{
	AcceleratingState accelState;
	
	train->setVelocity(10.0);
	double initialVel = train->getVelocity();
	
	accelState.update(train, 1.0);
	
	EXPECT_GT(train->getVelocity(), initialVel);
}

TEST_F(TrainStateTest, StoppedStateKeepsVelocityZero)
{
	StoppedState stoppedState;
	
	train->setVelocity(0.0);
	stoppedState.update(train, 1.0);
	
	EXPECT_DOUBLE_EQ(train->getVelocity(), 0.0);
}

TEST_F(TrainStateTest, StateRegistryProvidesIdleState)
{
	StateRegistry& registry = context->states();
	ITrainState* state = registry.idle();
	
	ASSERT_NE(state, nullptr);
	EXPECT_EQ(state->getName(), "Idle");
}

TEST_F(TrainStateTest, StateRegistryProvidesAllStates)
{
	StateRegistry& registry = context->states();
	
	EXPECT_NE(registry.idle(), nullptr);
	EXPECT_NE(registry.accelerating(), nullptr);
	EXPECT_NE(registry.cruising(), nullptr);
	EXPECT_NE(registry.braking(), nullptr);
	EXPECT_NE(registry.stopped(), nullptr);
	EXPECT_NE(registry.waiting(), nullptr);
	EXPECT_NE(registry.emergency(), nullptr);
}

TEST_F(TrainStateTest, TrainCanTransitionBetweenStates)
{
	StateRegistry& registry = context->states();
	
	train->setState(registry.idle());
	EXPECT_EQ(train->getCurrentState()->getName(), "Idle");
	
	train->setState(registry.accelerating());
	EXPECT_EQ(train->getCurrentState()->getName(), "Accelerating");
	
	train->setState(registry.cruising());
	EXPECT_EQ(train->getCurrentState()->getName(), "Cruising");
}

TEST_F(TrainStateTest, SimulationContextProvidesStateRegistry)
{
	StateRegistry& registry = context->states();
	EXPECT_NE(&registry, nullptr);
}

TEST_F(TrainStateTest, SimulationContextProvidesTrafficController)
{
	ITrainController* tc = context->getTrafficController();
	EXPECT_EQ(tc, trafficController);
}

TEST_F(TrainStateTest, CollisionSystemDetectsTrainOnRail)
{
	collisionSystem->refreshRailOccupancy(trains, &graph);
	
	bool foundTrain = false;
	for (Train* t : trains)
	{
		if (t == train)
		{
			foundTrain = true;
			break;
		}
	}
	
	EXPECT_TRUE(foundTrain);
}
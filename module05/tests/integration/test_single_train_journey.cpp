#include <gtest/gtest.h>
#include <iostream>
#include <iomanip>
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "patterns/factories/TrainFactory.hpp"
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "patterns/strategies/PathFinder.hpp"
#include "patterns/states/IdleState.hpp"
#include "patterns/states/AcceleratingState.hpp"
#include "patterns/states/CruisingState.hpp"
#include "patterns/states/BrakingState.hpp"
#include "patterns/states/StoppedState.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "utils/Time.hpp"

// Set to true to see terminal visualization
const bool ENABLE_VISUALIZATION = true;

class SingleTrainJourneyTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		Train::resetIDCounter();
		
		// Create simple network: A → B (50km, 250 km/h)
		nodeA = new Node("CityA");
		nodeB = new Node("CityB");
		
		graph.addNode(nodeA);
		graph.addNode(nodeB);
		
		rail = new Rail(nodeA, nodeB, 50.0, 250.0);
		graph.addRail(rail);
	}
	
	void TearDown() override
	{
		delete nodeA;
		delete nodeB;
		delete rail;
	}
	
	void visualize(double time, Train* train, Rail* currentRail)
	{
		if (!ENABLE_VISUALIZATION) return;
		
		std::cout << std::fixed << std::setprecision(2);
		std::cout << "[" << std::setw(6) << time << "s] ";
		std::cout << train->getCurrentState()->getName() << " | ";
		std::cout << "v=" << std::setw(6) << train->getVelocity() << " m/s | ";
		std::cout << "pos=" << std::setw(8) << train->getPosition() << " m | ";
		
		// ASCII rail visualization
		if (currentRail)
		{
			double railLengthM = PhysicsSystem::kmToM(currentRail->getLength());
			double progress = train->getPosition() / railLengthM;
			int barWidth = 30;
			int pos = static_cast<int>(progress * barWidth);
			
			std::cout << "[";
			for (int i = 0; i < barWidth; i++)
			{
				if (i == pos) std::cout << "🚅";
				else std::cout << "·";
			}
			std::cout << "]";
		}
		
		std::cout << std::endl;
	}
	
	Graph graph;
	Node* nodeA;
	Node* nodeB;
	Rail* rail;
};

// ===== BASIC JOURNEY TEST =====

TEST_F(SingleTrainJourneyTest, SimpleJourneyAtoB)
{
	// Create train
	TrainConfig config = {
		"Express", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(config, &graph);
	ASSERT_NE(train, nullptr);
	
	// Find path
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeB);
	ASSERT_EQ(path.size(), 1);
	train->setPath(path);
	
	// Set initial state
	AcceleratingState accelState;
	CruisingState cruisingState;
	BrakingState brakingState;
	StoppedState stoppedState(0.0);
	
	train->setState(&accelState);
	
	// Simulation parameters
	double dt = 1.0;  // 1 second timestep
	double time = 0.0;
	double maxTime = 1000.0;  // 1000 seconds max
	
	double railLengthM = PhysicsSystem::kmToM(rail->getLength());
	double speedLimitMs = PhysicsSystem::kmhToMs(rail->getSpeedLimit());
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n===== TRAIN JOURNEY: CityA → CityB =====\n";
		std::cout << "Rail: 50.0 km, Speed Limit: 250 km/h\n";
		std::cout << "Train: 80 tons, Max Accel: 356 kN, Max Brake: 500 kN\n\n";
	}
	
	bool reachedCruising = false;
	bool startedBraking = false;
	bool stopped = false;
	
	// Simulate journey
	while (time < maxTime && !stopped)
	{
		Rail* currentRail = train->getCurrentRail();
		
		// State transitions
		if (train->getCurrentState() == &accelState)
		{
			if (train->getVelocity() >= speedLimitMs * 0.99)
			{
				train->setState(&cruisingState);
				reachedCruising = true;
			}
		}
		else if (train->getCurrentState() == &cruisingState)
		{
			double brakingDist = PhysicsSystem::calculateBrakingDistance(train);
			double distRemaining = railLengthM - train->getPosition();
			
			if (distRemaining <= brakingDist * 1.1)
			{
				train->setState(&brakingState);
				startedBraking = true;
			}
		}
		else if (train->getCurrentState() == &brakingState)
		{
			if (train->getVelocity() <= 0.01 && train->getPosition() >= railLengthM * 0.99)
			{
				train->setState(&stoppedState);
				train->setVelocity(0.0);
				train->setPosition(railLengthM);
				stopped = true;
			}
		}
		
		// Update train
		train->update(dt);
		
		// Visualize every 10 seconds
		if (static_cast<int>(time) % 10 == 0)
		{
			visualize(time, train, currentRail);
		}
		
		time += dt;
	}
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n===== JOURNEY COMPLETE =====\n";
		std::cout << "Total time: " << time << " seconds (" << time/60.0 << " minutes)\n\n";
	}
	
	// Assertions
	EXPECT_TRUE(reachedCruising) << "Train should reach cruising speed";
	EXPECT_TRUE(startedBraking) << "Train should start braking";
	EXPECT_TRUE(stopped) << "Train should reach destination";
	EXPECT_NEAR(train->getPosition(), railLengthM, 10.0) << "Train should be at rail end";
	EXPECT_NEAR(train->getVelocity(), 0.0, 0.1) << "Train should be stopped";
	
	delete train;
}

// ===== STATE TRANSITION TEST =====

TEST_F(SingleTrainJourneyTest, StateTransitionsOccur)
{
	TrainConfig config = {
		"Express", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(config, &graph);
	ASSERT_NE(train, nullptr);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeB);
	train->setPath(path);
	
	IdleState idleState;
	AcceleratingState accelState;
	CruisingState cruisingState;
	
	// Test state transitions manually
	train->setState(&idleState);
	EXPECT_EQ(train->getCurrentState()->getName(), "Idle");
	
	train->setState(&accelState);
	EXPECT_EQ(train->getCurrentState()->getName(), "Accelerating");
	
	train->setState(&cruisingState);
	EXPECT_EQ(train->getCurrentState()->getName(), "Cruising");
	
	delete train;
}

// ===== PHYSICS INTEGRATION TEST =====

TEST_F(SingleTrainJourneyTest, PhysicsMovesTrainCorrectly)
{
	TrainConfig config = {
		"Express", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(config, &graph);
	ASSERT_NE(train, nullptr);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeB);
	train->setPath(path);
	
	AcceleratingState accelState;
	train->setState(&accelState);
	
	// Simulate 10 seconds of acceleration
	double initialPos = train->getPosition();
	double initialVel = train->getVelocity();
	
	for (int i = 0; i < 10; i++)
	{
		train->update(1.0);
	}
	
	// Train should have moved and gained velocity
	EXPECT_GT(train->getPosition(), initialPos) << "Train should move forward";
	EXPECT_GT(train->getVelocity(), initialVel) << "Train should accelerate";
	EXPECT_GT(train->getVelocity(), 40.0) << "Train should reach reasonable speed";
	
	delete train;
}
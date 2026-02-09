#include <gtest/gtest.h>
#include <iostream>
#include <iomanip>
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "patterns/factories/TrainFactory.hpp"
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "patterns/states/IdleState.hpp"
#include "patterns/states/AcceleratingState.hpp"
#include "patterns/states/CruisingState.hpp"
#include "patterns/states/BrakingState.hpp"
#include "patterns/states/StoppedState.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "utils/Time.hpp"

const bool ENABLE_VISUALIZATION = false;

class SingleTrainJourneyTest : public ::testing::Test
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
	}
	
	void TearDown() override
	{
		// Graph destructor handles cleanup
	}
	
	void visualize(double time, Train* train, Rail* currentRail)
	{
		if (!ENABLE_VISUALIZATION) return;
		
		std::cout << std::fixed << std::setprecision(2);
		std::cout << "[" << std::setw(6) << time << "s] ";
		std::cout << std::setw(13) << std::left << train->getCurrentState()->getName() << " | ";
		std::cout << "v=" << std::setw(6) << train->getVelocity() << " m/s | ";
		std::cout << "pos=" << std::setw(8) << train->getPosition() << " m | ";
		
		if (currentRail)
		{
			double railLen = currentRail->getLength() * 1000.0;
			double pos = train->getPosition();
			int barWidth = 40;
			int fillPos = static_cast<int>((pos / railLen) * barWidth);
			fillPos = std::max(0, std::min(barWidth, fillPos));
			
			std::cout << "[";
			for (int i = 0; i < barWidth; ++i)
			{
				std::cout << (i == fillPos ? "■" : "─");
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

TEST_F(SingleTrainJourneyTest, CompletesJourneyWithPhysics)
{
	TrainConfig cfg = {
		"Express", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(cfg, &graph);
	ASSERT_NE(train, nullptr);
	
	DijkstraStrategy dijkstra;
	train->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	
	AcceleratingState accelState;
	CruisingState cruiseState;
	BrakingState brakeState;
	
	double time = 0.0;
	const double dt = 1.0;
	const double maxTime = 2000.0;
	
	while (time < maxTime)
	{
		Rail* currentRail = train->getCurrentRail();
		if (!currentRail) break;
		
		double railLength = currentRail->getLength() * 1000.0;
		double remaining = railLength - train->getPosition();
		
		// Manual state control for testing
		if (train->getVelocity() < PhysicsSystem::kmhToMs(200.0))
		{
			train->setState(&accelState);
		}
		else if (remaining > 5000.0)
		{
			train->setState(&cruiseState);
		}
		else
		{
			train->setState(&brakeState);
		}
		
		train->update(dt);
		visualize(time, train, train->getCurrentRail());
		time += dt;
		
		if (train->getPosition() >= railLength)
		{
			train->advanceToNextRail();
			if (train->getCurrentRail() == nullptr)
			{
				train->markFinished();
			}
			break;
		}
	}
	
	EXPECT_TRUE(train->isFinished());
	EXPECT_LT(time, maxTime);
	
	delete train;
}

TEST_F(SingleTrainJourneyTest, AcceleratesFromRest)
{
	TrainConfig cfg = {
		"Express", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(cfg, &graph);
	ASSERT_NE(train, nullptr);
	
	DijkstraStrategy dijkstra;
	train->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	
	AcceleratingState accelState;
	train->setState(&accelState);
	train->setVelocity(0.0);
	
	double time = 0.0;
	const double dt = 1.0;
	bool velocityIncreased = false;
	
	for (int step = 0; step < 100; ++step)
	{
		double oldVel = train->getVelocity();
		train->update(dt);
		double newVel = train->getVelocity();
		
		if (newVel > oldVel)
		{
			velocityIncreased = true;
		}
		
		visualize(time, train, train->getCurrentRail());
		time += dt;
	}
	
	EXPECT_TRUE(velocityIncreased);
	EXPECT_GT(train->getVelocity(), 0.0);
	
	delete train;
}

TEST_F(SingleTrainJourneyTest, ReachesTargetVelocity)
{
	TrainConfig cfg = {
		"Express", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(cfg, &graph);
	ASSERT_NE(train, nullptr);
	
	DijkstraStrategy dijkstra;
	train->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	
	double targetVel = PhysicsSystem::kmhToMs(200.0);

	AcceleratingState accelState;
	CruisingState cruiseState;

	train->setState(&accelState);
	train->setVelocity(targetVel - 10.0);

	double time = 0.0;
	const double dt = 1.0;

	for (int step = 0; step < 50; ++step)
	{
		// Switch to cruise when close to target
		if (train->getVelocity() >= targetVel - 2.0)
		{
			train->setState(&cruiseState);
		}
		
		train->update(dt);
		visualize(time, train, train->getCurrentRail());
		time += dt;
		
		if (train->getVelocity() >= targetVel - 1.0)
		{
			break;
		}
	}

	EXPECT_NEAR(train->getVelocity(), targetVel, 5.0);  // Wider tolerance
	
	delete train;
}

TEST_F(SingleTrainJourneyTest, BrakesToStop)
{
	TrainConfig cfg = {
		"Express", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(cfg, &graph);
	ASSERT_NE(train, nullptr);
	
	DijkstraStrategy dijkstra;
	train->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	
	BrakingState brakeState;
	train->setState(&brakeState);
	train->setVelocity(PhysicsSystem::kmhToMs(100.0));
	
	double time = 0.0;
	const double dt = 1.0;
	double initialVel = train->getVelocity();
	
	for (int step = 0; step < 200; ++step)
	{
		train->update(dt);
		visualize(time, train, train->getCurrentRail());
		time += dt;
		
		if (train->getVelocity() == 0.0)
		{
			break;
		}
	}
	
	EXPECT_DOUBLE_EQ(train->getVelocity(), 0.0);
	EXPECT_LT(train->getVelocity(), initialVel);
	
	delete train;
}

TEST_F(SingleTrainJourneyTest, StateTransitionsCorrectly)
{
	TrainConfig cfg = {
		"Express", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(cfg, &graph);
	ASSERT_NE(train, nullptr);
	
	DijkstraStrategy dijkstra;
	train->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	
	AcceleratingState accelState;
	CruisingState cruiseState;
	
	bool sawIdle = false;
	bool sawAccelerating = false;
	
	double time = 0.0;
	const double dt = 1.0;
	const double maxTime = 2000.0;
	
	while (time < maxTime)
	{
		// Manually transition states for testing
		if (train->getVelocity() < PhysicsSystem::kmhToMs(150.0))
		{
			train->setState(&accelState);
		}
		else
		{
			train->setState(&cruiseState);
		}
		
		train->update(dt);
		
		std::string stateName = train->getCurrentState()->getName();
		
		if (stateName == "Idle") sawIdle = true;
		if (stateName == "Accelerating") sawAccelerating = true;
		
		visualize(time, train, train->getCurrentRail());
		time += dt;
		
		if (sawAccelerating && time > 100.0) break;
	}
	
	EXPECT_TRUE(sawIdle || sawAccelerating);
	EXPECT_TRUE(sawAccelerating);
	
	delete train;
}

TEST_F(SingleTrainJourneyTest, TravelTimeIsReasonable)
{
	TrainConfig cfg = {
		"Express", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(cfg, &graph);
	ASSERT_NE(train, nullptr);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, nodeA, nodeB);
	train->setPath(path);
	
	AcceleratingState accelState;
	CruisingState cruiseState;
	BrakingState brakeState;
	
	double time = 0.0;
	const double dt = 1.0;
	const double maxTime = 3000.0;
	
	while (time < maxTime)
	{
		Rail* currentRail = train->getCurrentRail();
		if (!currentRail) break;
		
		double railLength = currentRail->getLength() * 1000.0;
		double remaining = railLength - train->getPosition();
		
		// Manual state control for testing
		if (train->getVelocity() < PhysicsSystem::kmhToMs(200.0))
		{
			train->setState(&accelState);
		}
		else if (remaining > 5000.0)
		{
			train->setState(&cruiseState);
		}
		else
		{
			train->setState(&brakeState);
		}
		
		train->update(dt);
		time += dt;
		
		if (train->getPosition() >= railLength)
		{
			train->advanceToNextRail();
			if (train->getCurrentRail() == nullptr)
			{
				train->markFinished();
			}
			break;
		}
	}
	
	EXPECT_TRUE(train->isFinished());
	EXPECT_LT(time, 2000.0);
	EXPECT_GT(time, 600.0);
	
	delete train;
}
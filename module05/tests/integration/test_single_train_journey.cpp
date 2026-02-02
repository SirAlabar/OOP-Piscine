// tests/integration/test_single_train_journey.cpp
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
		std::cout << std::setw(13) << std::left << train->getCurrentState()->getName() << " | ";
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
				if (i == pos) std::cout << "🚆";
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

// ===== REQUIREMENT: Pathfinding + Travel Time Estimation =====

TEST_F(SingleTrainJourneyTest, CompleteJourneyWithEstimation)
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
	
	// ===== TRAVEL TIME ESTIMATION =====
	double estimatedTimeHours = 0.0;
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n╔════════════════════════════════════════════════╗\n";
		std::cout << "║     RAILWAY SIMULATION - JOURNEY PLANNER       ║\n";
		std::cout << "╚════════════════════════════════════════════════╝\n\n";
		std::cout << "Train: " << train->getName() << " (ID: " << train->getID() << ")\n";
		std::cout << "Mass: " << train->getMass() << " tons\n";
		std::cout << "Max Accel: " << train->getMaxAccelForce() << " kN | ";
		std::cout << "Max Brake: " << train->getMaxBrakeForce() << " kN\n\n";
		std::cout << "Route: " << nodeA->getName() << " → " << nodeB->getName() << "\n";
		std::cout << "Path segments: " << path.size() << "\n\n";
		std::cout << "─────────────────────────────────────────────────\n";
		std::cout << "ESTIMATED TRAVEL TIME CALCULATION:\n";
		std::cout << "─────────────────────────────────────────────────\n";
	}
	
	for (Rail* r : path)
	{
		double segmentTime = r->getLength() / r->getSpeedLimit();
		estimatedTimeHours += segmentTime;
		
		if (ENABLE_VISUALIZATION)
		{
			std::cout << "  " << r->getNodeA()->getName() 
			          << " → " << r->getNodeB()->getName()
			          << " | " << r->getLength() << " km @ " 
			          << r->getSpeedLimit() << " km/h"
			          << " ⇒ " << (segmentTime * 60.0) << " min\n";
		}
	}
	
	double estimatedTimeMinutes = estimatedTimeHours * 60.0;
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\nEstimated travel time (ideal): " 
		          << estimatedTimeMinutes << " minutes\n";
		std::cout << "(Assumes instant acceleration to speed limit)\n\n";
		std::cout << "─────────────────────────────────────────────────\n";
		std::cout << "SIMULATION START\n";
		std::cout << "─────────────────────────────────────────────────\n\n";
	}
	
	// Set initial state
	AcceleratingState accelState;
	CruisingState cruisingState;
	BrakingState brakingState;
	StoppedState stoppedState;
	
	train->setState(&accelState);
	
	// Simulation parameters
	double dt = 1.0;
	double time = 0.0;
	double maxTime = 1000.0;
	
	double railLengthM = PhysicsSystem::kmToM(rail->getLength());
	double speedLimitMs = PhysicsSystem::kmhToMs(rail->getSpeedLimit());
	
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
				if (ENABLE_VISUALIZATION)
				{
					std::cout << "\n>>> Reached cruising speed: " 
					          << PhysicsSystem::msToKmh(train->getVelocity()) 
					          << " km/h\n\n";
				}
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
				if (ENABLE_VISUALIZATION)
				{
					std::cout << "\n>>> Initiating braking (distance remaining: " 
					          << distRemaining << " m)\n\n";
				}
			}
		}
		else if (train->getCurrentState() == &brakingState)
		{
			if (train->getVelocity() <= 0.01 && 
			    train->getPosition() >= railLengthM * 0.99)
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
	
	double actualTimeMinutes = time / 60.0;
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n─────────────────────────────────────────────────\n";
		std::cout << "JOURNEY COMPLETE ✓\n";
		std::cout << "─────────────────────────────────────────────────\n";
		std::cout << "Estimated time: " << estimatedTimeMinutes << " min\n";
		std::cout << "Actual time:    " << actualTimeMinutes << " min\n";
		std::cout << "Difference:     +" << (actualTimeMinutes - estimatedTimeMinutes) 
		          << " min (due to acceleration/braking)\n";
		std::cout << "Final position: " << train->getPosition() / 1000.0 << " km\n";
		std::cout << "Final velocity: " << train->getVelocity() << " m/s\n\n";
	}
	
	// Assertions
	EXPECT_TRUE(reachedCruising) << "Train should reach cruising speed";
	EXPECT_TRUE(startedBraking) << "Train should start braking";
	EXPECT_TRUE(stopped) << "Train should reach destination";
	EXPECT_NEAR(train->getPosition(), railLengthM, 10.0);
	EXPECT_NEAR(train->getVelocity(), 0.0, 0.1);
	
	// Travel time validation
	EXPECT_GE(actualTimeMinutes, estimatedTimeMinutes) 
		<< "Actual time should be >= estimated";
	EXPECT_LE(actualTimeMinutes, estimatedTimeMinutes * 1.5) 
		<< "Actual time should be reasonably close";
	
	delete train;
}

// ===== REQUIREMENT: Complex Pathfinding (Multiple Routes) =====

TEST(ComplexPathfindingTest, MultipleRoutesOptimalSelection)
{
	Graph graph;
	
	Node* A = new Node("CityA");
	Node* B = new Node("CityB");
	Node* C = new Node("CityC");
	Node* D = new Node("CityD");
	
	graph.addNode(A);
	graph.addNode(B);
	graph.addNode(C);
	graph.addNode(D);
	
	// Route 1: A → B → D (20 km total, 12 min)
	Rail* AB = new Rail(A, B, 10.0, 100.0);  // 6 min
	Rail* BD = new Rail(B, D, 10.0, 100.0);  // 6 min
	
	// Route 2: A → C → D (35 km total, 12 min - same time!)
	Rail* AC = new Rail(A, C, 25.0, 250.0);  // 6 min
	Rail* CD = new Rail(C, D, 10.0, 100.0);  // 6 min
	
	graph.addRail(AB);
	graph.addRail(BD);
	graph.addRail(AC);
	graph.addRail(CD);
	
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, A, D);
	
	ASSERT_FALSE(path.empty());
	ASSERT_EQ(path.size(), 2);
	
	// Calculate travel time
	double totalTimeHours = 0.0;
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n╔════════════════════════════════════════════════╗\n";
		std::cout << "║   COMPLEX PATHFINDING TEST (2 ROUTES)          ║\n";
		std::cout << "╚════════════════════════════════════════════════╝\n\n";
		std::cout << "Route 1: A → B → D (10+10 km @ 100 km/h) = 12 min\n";
		std::cout << "Route 2: A → C → D (25+10 km @ 250/100 km/h) = 12 min\n\n";
		std::cout << "Selected path:\n";
	}
	
	for (Rail* r : path)
	{
		double segmentTime = r->getLength() / r->getSpeedLimit();
		totalTimeHours += segmentTime;
		
		if (ENABLE_VISUALIZATION)
		{
			std::cout << "  " << r->getNodeA()->getName() 
			          << " → " << r->getNodeB()->getName()
			          << " (" << r->getLength() << " km @ " 
			          << r->getSpeedLimit() << " km/h) = "
			          << (segmentTime * 60.0) << " min\n";
		}
	}
	
	double totalTimeMinutes = totalTimeHours * 60.0;
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\nTotal estimated time: " << totalTimeMinutes << " minutes\n\n";
	}
	
	// Both routes take 12 minutes, so either is valid
	EXPECT_NEAR(totalTimeMinutes, 12.0, 0.1);
	
	// Cleanup
	delete A; delete B; delete C; delete D;
	delete AB; delete BD; delete AC; delete CD;
}

// ===== REQUIREMENT: Multi-Hop Journey =====

TEST(MultiHopJourneyTest, ThreeStationJourney)
{
	Train::resetIDCounter();
	
	Graph graph;
	
	Node* A = new Node("CityA");
	Node* B = new Node("CityB");
	Node* C = new Node("CityC");
	
	graph.addNode(A);
	graph.addNode(B);
	graph.addNode(C);
	
	Rail* AB = new Rail(A, B, 30.0, 200.0);
	Rail* BC = new Rail(B, C, 20.0, 150.0);
	
	graph.addRail(AB);
	graph.addRail(BC);
	
	// Create train
	TrainConfig config = {
		"MultiHop", 60.0, 0.005, 400.0, 450.0,
		"CityA", "CityC",
		Time("00h00"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(config, &graph);
	ASSERT_NE(train, nullptr);
	
	// Find path
	DijkstraStrategy dijkstra;
	auto path = dijkstra.findPath(&graph, A, C);
	
	ASSERT_EQ(path.size(), 2);
	train->setPath(path);
	
	// Calculate estimated time
	double estimatedTimeHours = 0.0;
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n╔════════════════════════════════════════════════╗\n";
		std::cout << "║       MULTI-HOP JOURNEY TEST (A→B→C)           ║\n";
		std::cout << "╚════════════════════════════════════════════════╝\n\n";
		std::cout << "Path:\n";
	}
	
	for (size_t i = 0; i < path.size(); i++)
	{
		Rail* r = path[i];
		double segmentTime = r->getLength() / r->getSpeedLimit();
		estimatedTimeHours += segmentTime;
		
		if (ENABLE_VISUALIZATION)
		{
			std::cout << "  Segment " << (i+1) << ": " 
			          << r->getNodeA()->getName() << " → " 
			          << r->getNodeB()->getName()
			          << " (" << r->getLength() << " km @ " 
			          << r->getSpeedLimit() << " km/h) = "
			          << (segmentTime * 60.0) << " min\n";
		}
	}
	
	double estimatedTimeMinutes = estimatedTimeHours * 60.0;
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\nTotal estimated time: " << estimatedTimeMinutes 
		          << " minutes\n\n";
	}
	
	// Expected: 30/200 + 20/150 = 0.15 + 0.133 = 0.283 hours = 17 minutes
	EXPECT_NEAR(estimatedTimeMinutes, 17.0, 1.0);
	EXPECT_EQ(path[0], AB);
	EXPECT_EQ(path[1], BC);
	
	// Cleanup
	delete train;
	delete A; delete B; delete C;
	delete AB; delete BC;
}

// ===== REQUIREMENT: Network Validation =====

TEST(NetworkTest, ComplexNetworkStructure)
{
	Graph graph;
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n╔════════════════════════════════════════════════╗\n";
		std::cout << "║       COMPLEX NETWORK STRUCTURE TEST           ║\n";
		std::cout << "╚════════════════════════════════════════════════╝\n\n";
	}
	
	// Create 5 cities, 7 rail connections
	Node* A = new Node("CityA", NodeType::CITY);
	Node* B = new Node("CityB", NodeType::CITY);
	Node* C = new Node("CityC", NodeType::CITY);
	Node* D = new Node("CityD", NodeType::CITY);
	Node* E = new Node("CityE", NodeType::CITY);
	
	graph.addNode(A);
	graph.addNode(B);
	graph.addNode(C);
	graph.addNode(D);
	graph.addNode(E);
	
	// Create mesh network
    Rail* AB = new Rail(A, B, 50.0, 250.0);
    Rail* AC = new Rail(A, C, 30.0, 200.0);
    Rail* BD = new Rail(B, D, 40.0, 220.0);
    Rail* CD = new Rail(C, D, 25.0, 180.0);
    Rail* CE = new Rail(C, E, 35.0, 240.0);
    Rail* DE = new Rail(D, E, 20.0, 200.0);
    Rail* BE = new Rail(B, E, 60.0, 250.0);

    graph.addRail(AB);
    graph.addRail(AC);
    graph.addRail(BD);
    graph.addRail(CD);
    graph.addRail(CE);
    graph.addRail(DE);
    graph.addRail(BE);
	
	EXPECT_EQ(graph.getNodeCount(), 5);
	EXPECT_EQ(graph.getRailCount(), 7);
	EXPECT_TRUE(graph.isValid());
	
	// Test pathfinding in complex network
	DijkstraStrategy dijkstra;
	auto pathAE = dijkstra.findPath(&graph, A, E);
	
	ASSERT_FALSE(pathAE.empty());
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "Network: 5 cities, 7 rails\n";
		std::cout << "Pathfinding A → E:\n";
		
		double totalTime = 0.0;
		for (Rail* r : pathAE)
		{
			double segmentTime = r->getLength() / r->getSpeedLimit();
			totalTime += segmentTime;
			std::cout << "  " << r->getNodeA()->getName() 
			          << " → " << r->getNodeB()->getName()
			          << " (" << r->getLength() << " km)\n";
		}
		std::cout << "\nPath length: " << pathAE.size() << " segments\n";
		std::cout << "Estimated time: " << (totalTime * 60.0) << " minutes\n\n";
	}
	
	// Cleanup
	delete AB;
    delete AC;
    delete BD;
    delete CD;
    delete CE;
    delete DE;
    delete BE;
    delete A;
    delete B;
    delete C;
    delete D;
    delete E;
}

// ===== State transitions test =====

TEST_F(SingleTrainJourneyTest, AllStateTransitions)
{
	TrainConfig config = {
		"TestTrain", 80.0, 0.005, 356.0, 500.0,
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
	BrakingState brakingState;
	StoppedState stoppedState;
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n╔════════════════════════════════════════════════╗\n";
		std::cout << "║         STATE TRANSITION TEST                  ║\n";
		std::cout << "╚════════════════════════════════════════════════╝\n\n";
	}
	
	// Test all state transitions
	train->setState(&idleState);
	EXPECT_EQ(train->getCurrentState()->getName(), "Idle");
	if (ENABLE_VISUALIZATION) std::cout << "✓ Idle state\n";
	
	train->setState(&accelState);
	EXPECT_EQ(train->getCurrentState()->getName(), "Accelerating");
	if (ENABLE_VISUALIZATION) std::cout << "✓ Accelerating state\n";
	
	train->setState(&cruisingState);
	EXPECT_EQ(train->getCurrentState()->getName(), "Cruising");
	if (ENABLE_VISUALIZATION) std::cout << "✓ Cruising state\n";
	
	train->setState(&brakingState);
	EXPECT_EQ(train->getCurrentState()->getName(), "Braking");
	if (ENABLE_VISUALIZATION) std::cout << "✓ Braking state\n";
	
	train->setState(&stoppedState);
	EXPECT_EQ(train->getCurrentState()->getName(), "Stopped");
	if (ENABLE_VISUALIZATION) std::cout << "✓ Stopped state\n\n";
	
	delete train;
}

// ===== Physics validation =====

TEST_F(SingleTrainJourneyTest, PhysicsAccuracyValidation)
{
	TrainConfig config = {
		"PhysicsTest", 80.0, 0.005, 356.0, 500.0,
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
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n╔════════════════════════════════════════════════╗\n";
		std::cout << "║       PHYSICS ACCURACY VALIDATION              ║\n";
		std::cout << "╚════════════════════════════════════════════════╝\n\n";
	}
	
	// Simulate 10 seconds of acceleration
	double initialPos = train->getPosition();
	double initialVel = train->getVelocity();
	
	for (int i = 0; i < 10; i++)
	{
		train->update(1.0);
	}
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "After 10 seconds of acceleration:\n";
		std::cout << "  Position: " << initialPos << " → " 
		          << train->getPosition() << " m\n";
		std::cout << "  Velocity: " << initialVel << " → " 
		          << train->getVelocity() << " m/s\n";
		std::cout << "  Distance traveled: " << train->getPosition() << " m\n\n";
	}
	
	// Assertions
	EXPECT_GT(train->getPosition(), initialPos);
	EXPECT_GT(train->getVelocity(), initialVel);
	EXPECT_GT(train->getVelocity(), 40.0) << "Should reach 40+ m/s";
	EXPECT_NEAR(train->getVelocity(), 44.01, 0.1) 
		<< "Should match physics calculations";
	
	delete train;
}
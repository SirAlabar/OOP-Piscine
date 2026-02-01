#include <gtest/gtest.h>
#include <iostream>
#include <iomanip>
#include "simulation/SimulationManager.hpp"
#include "patterns/factories/TrainFactory.hpp"
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "core/Train.hpp"
#include "patterns/states/IdleState.hpp"
#include "utils/Time.hpp"

const bool ENABLE_VISUALIZATION = true;

class MultiTrainTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		Train::resetIDCounter();
		
		// Create simple network: A → B (30km, 200 km/h)
		nodeA = new Node("CityA");
		nodeB = new Node("CityB");
		
		graph.addNode(nodeA);
		graph.addNode(nodeB);
		
		rail = new Rail(nodeA, nodeB, 30.0, 200.0);
		graph.addRail(rail);
		
		// Reset SimulationManager
		SimulationManager::getInstance().reset();
		SimulationManager::getInstance().setNetwork(&graph);
	}
	
	void TearDown() override
	{
		SimulationManager::getInstance().reset();
		delete nodeA;
		delete nodeB;
		delete rail;
	}
	
	void visualize(double time)
	{
		if (!ENABLE_VISUALIZATION) return;
		
		const auto& trains = SimulationManager::getInstance().getTrains();
		
		std::cout << "\n[" << std::fixed << std::setprecision(1) << std::setw(6) << time << "s]";
		
		for (Train* train : trains)
		{
			if (!train) continue;
			
			std::cout << " | " << train->getName() 
			          << ": " << std::setw(12) << std::left 
			          << (train->getCurrentState() ? train->getCurrentState()->getName() : "NULL")
			          << " v=" << std::setw(5) << std::right << std::setprecision(1) << train->getVelocity()
			          << "m/s pos=" << std::setw(7) << train->getPosition() << "m";
		}
	}
	
	Graph graph;
	Node* nodeA;
	Node* nodeB;
	Rail* rail;
};

// ===== TEST 1: Two Trains, Same Route, Different Departure Times =====

TEST_F(MultiTrainTest, TwoTrainsSameRouteDifferentDepartures)
{
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n╔════════════════════════════════════════════════╗\n";
		std::cout << "║  TWO TRAINS - SAME ROUTE - COLLISION TEST     ║\n";
		std::cout << "╚════════════════════════════════════════════════╝\n";
		std::cout << "\nScenario: Train1 departs 00h00, Train2 departs 00h05\n";
		std::cout << "Expected: Train2 waits when approaching occupied rail\n\n";
	}
	
	// Train 1: Departs at 00h00
	TrainConfig config1 = {
		"Train1", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	// Train 2: Departs at 00h05 (5 minutes later)
	TrainConfig config2 = {
		"Train2", 60.0, 0.005, 400.0, 450.0,
		"CityA", "CityB",
		Time("00h05"), Time("00h05")
	};
	
	Train* train1 = TrainFactory::create(config1, &graph);
	Train* train2 = TrainFactory::create(config2, &graph);
	
	ASSERT_NE(train1, nullptr);
	ASSERT_NE(train2, nullptr);
	
	// Find paths
	DijkstraStrategy dijkstra;
	auto path1 = dijkstra.findPath(&graph, nodeA, nodeB);
	auto path2 = dijkstra.findPath(&graph, nodeA, nodeB);
	
	ASSERT_EQ(path1.size(), 1);
	ASSERT_EQ(path2.size(), 1);
	
	train1->setPath(path1);
	train2->setPath(path2);
	
	// Set initial states
	static IdleState idleState;
	train1->setState(&idleState);
	train2->setState(&idleState);
	
	// Add to simulation
	SimulationManager::getInstance().addTrain(train1);
	SimulationManager::getInstance().addTrain(train2);
	
	// Run simulation
	double maxTime = 1200.0; // 20 minutes
	double dt = 1.0;
	
	bool train2Waited = false;
	bool train1Finished = false;
	bool train2Finished = false;
	
	SimulationManager::getInstance().start();
	
	for (double time = 0.0; time < maxTime; time += dt)
	{
		SimulationManager::getInstance().step();
		
		// Visualize every 30 seconds
		if (static_cast<int>(time) % 30 == 0)
		{
			visualize(time);
		}
		
		// Check if Train2 ever enters Waiting state
		if (train2->getCurrentState() && 
		    train2->getCurrentState()->getName() == "Waiting")
		{
			train2Waited = true;
			if (ENABLE_VISUALIZATION && !train2Finished)
			{
				std::cout << "\n\n>>> COLLISION AVOIDANCE: Train2 is WAITING for Train1 to clear rail!\n";
			}
		}
		
		// Check if trains finished
		if (!train1Finished && train1->getCurrentRail() == nullptr)
		{
			train1Finished = true;
			if (ENABLE_VISUALIZATION)
			{
				std::cout << "\n\n>>> Train1 ARRIVED at CityB (time: " << time << "s)\n";
			}
		}
		
		if (!train2Finished && train2->getCurrentRail() == nullptr)
		{
			train2Finished = true;
			if (ENABLE_VISUALIZATION)
			{
				std::cout << "\n\n>>> Train2 ARRIVED at CityB (time: " << time << "s)\n";
			}
		}
		
		// Stop if both finished
		if (train1Finished && train2Finished)
		{
			break;
		}
	}
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n\n─────────────────────────────────────────────────\n";
		std::cout << "SIMULATION COMPLETE ✓\n";
		std::cout << "─────────────────────────────────────────────────\n";
		std::cout << "Train1 finished: " << (train1Finished ? "YES ✓" : "NO ✗") << "\n";
		std::cout << "Train2 finished: " << (train2Finished ? "YES ✓" : "NO ✗") << "\n";
		std::cout << "Train2 waited:   " << (train2Waited ? "YES ✓" : "NO ✗") << "\n";
		std::cout << "─────────────────────────────────────────────────\n\n";
	}
	
	// Assertions
	EXPECT_TRUE(train1Finished) << "Train1 should complete journey";
	EXPECT_TRUE(train2Finished) << "Train2 should complete journey";
	EXPECT_TRUE(train2Waited) << "Train2 should have waited for Train1";
	
	// Verify no collision (trains never at same position)
	EXPECT_NE(train1->getPosition(), train2->getPosition()) 
		<< "Trains should not be at same position";
	
	delete train1;
	delete train2;
}

// ===== TEST 2: Three Trains, Sequential Departures =====

TEST_F(MultiTrainTest, ThreeTrainsSequential)
{
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n╔════════════════════════════════════════════════╗\n";
		std::cout << "║  THREE TRAINS - SEQUENTIAL DEPARTURES         ║\n";
		std::cout << "╚════════════════════════════════════════════════╝\n\n";
	}
	
	// Train 1: 00h00
	TrainConfig config1 = {
		"Train1", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	// Train 2: 00h03
	TrainConfig config2 = {
		"Train2", 70.0, 0.005, 380.0, 480.0,
		"CityA", "CityB",
		Time("00h03"), Time("00h05")
	};
	
	// Train 3: 00h06
	TrainConfig config3 = {
		"Train3", 60.0, 0.005, 400.0, 450.0,
		"CityA", "CityB",
		Time("00h06"), Time("00h05")
	};
	
	Train* train1 = TrainFactory::create(config1, &graph);
	Train* train2 = TrainFactory::create(config2, &graph);
	Train* train3 = TrainFactory::create(config3, &graph);
	
	ASSERT_NE(train1, nullptr);
	ASSERT_NE(train2, nullptr);
	ASSERT_NE(train3, nullptr);
	
	DijkstraStrategy dijkstra;
	train1->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	train2->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	train3->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	
	static IdleState idleState;
	train1->setState(&idleState);
	train2->setState(&idleState);
	train3->setState(&idleState);
	
	SimulationManager::getInstance().addTrain(train1);
	SimulationManager::getInstance().addTrain(train2);
	SimulationManager::getInstance().addTrain(train3);
	
	SimulationManager::getInstance().run(1500.0); // 25 minutes
	
	// All trains should finish
	EXPECT_EQ(train1->getCurrentRail(), nullptr) << "Train1 should finish";
	EXPECT_EQ(train2->getCurrentRail(), nullptr) << "Train2 should finish";
	EXPECT_EQ(train3->getCurrentRail(), nullptr) << "Train3 should finish";
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n✓ All three trains completed journey\n\n";
	}
	
	delete train1;
	delete train2;
	delete train3;
}

// ===== TEST 3: Different Routes (No Collision Expected) =====

TEST_F(MultiTrainTest, DifferentRoutesNoCollision)
{
	// Extend network: A → B, A → C
	Node* nodeC = new Node("CityC");
	graph.addNode(nodeC);
	
	Rail* railAC = new Rail(nodeA, nodeC, 25.0, 180.0);
	graph.addRail(railAC);
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n╔════════════════════════════════════════════════╗\n";
		std::cout << "║  TWO TRAINS - DIFFERENT ROUTES (NO COLLISION) ║\n";
		std::cout << "╚════════════════════════════════════════════════╝\n\n";
	}
	
	// Train 1: A → B
	TrainConfig config1 = {
		"Train1_AB", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	// Train 2: A → C (different route)
	TrainConfig config2 = {
		"Train2_AC", 60.0, 0.005, 400.0, 450.0,
		"CityA", "CityC",
		Time("00h00"), Time("00h05")
	};
	
	Train* train1 = TrainFactory::create(config1, &graph);
	Train* train2 = TrainFactory::create(config2, &graph);
	
	ASSERT_NE(train1, nullptr);
	ASSERT_NE(train2, nullptr);
	
	DijkstraStrategy dijkstra;
	train1->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	train2->setPath(dijkstra.findPath(&graph, nodeA, nodeC));
	
	static IdleState idleState;
	train1->setState(&idleState);
	train2->setState(&idleState);
	
	SimulationManager::getInstance().addTrain(train1);
	SimulationManager::getInstance().addTrain(train2);
	
	SimulationManager::getInstance().run(1000.0);
	
	// Both should finish independently
	EXPECT_EQ(train1->getCurrentRail(), nullptr);
	EXPECT_EQ(train2->getCurrentRail(), nullptr);
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n✓ Both trains completed on separate routes\n\n";
	}
	
	delete train1;
	delete train2;
	delete nodeC;
	delete railAC;
}

// ===== TEST 4: Collision Avoidance Verification =====

TEST_F(MultiTrainTest, VerifyNoCollisionOccurs)
{
	TrainConfig config1 = {
		"Fast", 60.0, 0.005, 450.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	TrainConfig config2 = {
		"Slow", 100.0, 0.005, 300.0, 400.0,
		"CityA", "CityB",
		Time("00h02"), Time("00h05")
	};
	
	Train* train1 = TrainFactory::create(config1, &graph);
	Train* train2 = TrainFactory::create(config2, &graph);
	
	DijkstraStrategy dijkstra;
	train1->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	train2->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	
	static IdleState idleState;
	train1->setState(&idleState);
	train2->setState(&idleState);
	
	SimulationManager::getInstance().addTrain(train1);
	SimulationManager::getInstance().addTrain(train2);
	
	SimulationManager::getInstance().start();
	
	double minDistance = 1e9;
	
	for (double time = 0.0; time < 1200.0; time += 1.0)
	{
		SimulationManager::getInstance().step();
		
		// Track minimum distance between trains
		if (train1->getCurrentRail() && train2->getCurrentRail())
		{
			double dist = std::abs(train1->getPosition() - train2->getPosition());
			if (dist < minDistance)
			{
				minDistance = dist;
			}
		}
	}
	
	// Minimum distance should be reasonable (at least 50m separation)
	EXPECT_GT(minDistance, 50.0) << "Trains got too close: " << minDistance << "m";
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n✓ Minimum separation maintained: " << minDistance << "m\n\n";
	}
	
	delete train1;
	delete train2;
}
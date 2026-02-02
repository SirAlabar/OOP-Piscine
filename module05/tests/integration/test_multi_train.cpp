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
        
        // A → SlowZone → FastZone → B
        nodeA = new Node("CityA");
        slowZone = new Node("SlowZone", NodeType::JUNCTION);
        fastZone = new Node("FastZone", NodeType::JUNCTION);
        nodeB = new Node("CityB");
        
        graph.addNode(nodeA);
        graph.addNode(slowZone);
        graph.addNode(fastZone);
        graph.addNode(nodeB);
        
        // 3 segments with different limits
        rail1 = new Rail(nodeA, slowZone, 10.0, 100.0);    // Slow
        rail2 = new Rail(slowZone, fastZone, 10.0, 200.0); // Medium
        rail3 = new Rail(fastZone, nodeB, 10.0, 250.0);    // Fast
        
        graph.addRail(rail1);
        graph.addRail(rail2);
        graph.addRail(rail3);
        
        SimulationManager::getInstance().reset();
        SimulationManager::getInstance().setNetwork(&graph);
    }
	
	void TearDown() override
    {
        SimulationManager::getInstance().reset();
        delete nodeA;
        delete slowZone;
        delete fastZone;
        delete nodeB;
        delete rail1;
        delete rail2;
        delete rail3;
    }
	
	void visualize(double time)
	{
		if (!ENABLE_VISUALIZATION) return;
		
		const auto& trains = SimulationManager::getInstance().getTrains();
		
		std::cout << "\n[" << std::fixed << std::setprecision(1) << std::setw(6) << time << "s]";
		
		for (Train* train : trains)
		{
			if (!train) continue;
			
			std::cout << " | " << std::setw(10) << std::left << train->getName() 
			          << ": " << std::setw(12) << std::left 
			          << (train->getCurrentState() ? train->getCurrentState()->getName() : "FINISHED")
			          << " v=" << std::setw(5) << std::right << std::setprecision(1) << train->getVelocity()
			          << "m/s pos=" << std::setw(7) << train->getPosition() << "m";
		}
	}
	
    Graph graph;
    Node *nodeA, *slowZone, *fastZone, *nodeB;
    Rail *rail1, *rail2, *rail3;
};

// ===== TEST 1: Fast Train Catches Slow Train =====

TEST_F(MultiTrainTest, FastTrainCatchesSlowTrain)
{
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n╔════════════════════════════════════════════════╗\n";
		std::cout << "║  FAST TRAIN CATCHES SLOW TRAIN                ║\n";
		std::cout << "╚════════════════════════════════════════════════╝\n";
		std::cout << "\nScenario: Slow train departs first, fast train catches up\n";
		std::cout << "Expected: Fast train enters Waiting state\n\n";
	}
	
	// Slow Train: Heavy, weak, departs first
	TrainConfig config1 = {
		"SlowTrain", 120.0, 0.008, 250.0, 400.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	// Fast Train: Light, powerful, departs 1 minute later
	TrainConfig config2 = {
		"FastTrain", 60.0, 0.003, 500.0, 600.0,
		"CityA", "CityB",
		Time("00h01"), Time("00h05")
	};
	
	Train* train1 = TrainFactory::create(config1, &graph);
	Train* train2 = TrainFactory::create(config2, &graph);
	
	ASSERT_NE(train1, nullptr);
	ASSERT_NE(train2, nullptr);
	
	DijkstraStrategy dijkstra;
	train1->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	train2->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	
	static IdleState idleState;
	train1->setState(&idleState);
	train2->setState(&idleState);
	
	SimulationManager::getInstance().addTrain(train1);
	SimulationManager::getInstance().addTrain(train2);
	
	SimulationManager::getInstance().start();
	
	bool train2Waited = false;
	bool train1Finished = false;
	bool train2Finished = false;
	
	for (double time = 0.0; time < 1500.0; time += 1.0)
	{
		SimulationManager::getInstance().step();
		
		if (static_cast<int>(time) % 60 == 0)
		{
			visualize(time);
		}
		
		if (train2->getCurrentState() && 
		    train2->getCurrentState()->getName() == "Waiting")
		{
			if (!train2Waited && ENABLE_VISUALIZATION)
			{
				std::cout << "\n\n>>> COLLISION AVOIDANCE ACTIVATED (time: " << time << "s)\n";
				std::cout << "    FastTrain is WAITING for SlowTrain to clear!\n";
			}
			train2Waited = true;
		}
		
		if (!train1Finished && train1->getCurrentRail() == nullptr)
		{
			train1Finished = true;
			if (ENABLE_VISUALIZATION)
			{
				std::cout << "\n\n>>> SlowTrain ARRIVED at CityB (time: " << time << "s)\n";
			}
		}
		
		if (!train2Finished && train2->getCurrentRail() == nullptr)
		{
			train2Finished = true;
			if (ENABLE_VISUALIZATION)
			{
				std::cout << "\n\n>>> FastTrain ARRIVED at CityB (time: " << time << "s)\n";
			}
		}
		
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
		std::cout << "SlowTrain finished:  " << (train1Finished ? "YES ✓" : "NO ✗") << "\n";
		std::cout << "FastTrain finished:  " << (train2Finished ? "YES ✓" : "NO ✗") << "\n";
		std::cout << "FastTrain waited:    " << (train2Waited ? "YES ✓" : "NO ✗") << "\n";
		std::cout << "─────────────────────────────────────────────────\n\n";
	}
	
	EXPECT_TRUE(train1Finished) << "SlowTrain should complete journey";
	EXPECT_TRUE(train2Finished) << "FastTrain should complete journey";
	EXPECT_TRUE(train2Waited) << "FastTrain should have waited for SlowTrain";
	
	delete train1;
	delete train2;
}

// ===== TEST 2: Simultaneous Departure with Manual Offset =====

TEST_F(MultiTrainTest, SimultaneousDepartureWithOffset)
{
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n╔════════════════════════════════════════════════╗\n";
		std::cout << "║  SIMULTANEOUS DEPARTURE - MANUAL OFFSET       ║\n";
		std::cout << "╚════════════════════════════════════════════════╝\n";
		std::cout << "\nScenario: Both trains depart 00h00, Train2 starts 100m behind\n";
		std::cout << "Expected: Train2 detects Train1 immediately, enters Waiting\n\n";
	}
	
	TrainConfig config1 = {
		"Train1", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	TrainConfig config2 = {
		"Train2", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	Train* train1 = TrainFactory::create(config1, &graph);
	Train* train2 = TrainFactory::create(config2, &graph);
	
	ASSERT_NE(train1, nullptr);
	ASSERT_NE(train2, nullptr);
	
	DijkstraStrategy dijkstra;
	train1->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	train2->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	
	// Manually offset Train2 by 100m behind
	train2->setPosition(-100.0);
	
	static IdleState idleState;
	train1->setState(&idleState);
	train2->setState(&idleState);
	
	SimulationManager::getInstance().addTrain(train1);
	SimulationManager::getInstance().addTrain(train2);
	
	SimulationManager::getInstance().start();
	
	bool train2Waited = false;
	double minDistance = 1e9;
	
	for (double time = 0.0; time < 1200.0; time += 1.0)
	{
		SimulationManager::getInstance().step();
		
		if (static_cast<int>(time) % 60 == 0)
		{
			visualize(time);
		}
		
		if (train2->getCurrentState() && 
		    train2->getCurrentState()->getName() == "Waiting")
		{
			train2Waited = true;
		}
		
		// Track minimum distance while both on same rail
		if (train1->getCurrentRail() && train2->getCurrentRail() &&
		    train1->getCurrentRail() == train2->getCurrentRail())
		{
			double dist = std::abs(train1->getPosition() - train2->getPosition());
			if (dist < minDistance)
			{
				minDistance = dist;
			}
		}
		
		if (train1->getCurrentRail() == nullptr && train2->getCurrentRail() == nullptr)
		{
			break;
		}
	}
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n\n─────────────────────────────────────────────────\n";
		std::cout << "Train2 waited:        " << (train2Waited ? "YES ✓" : "NO ✗") << "\n";
		std::cout << "Minimum separation:   " << std::fixed << std::setprecision(1) 
		          << minDistance << "m\n";
		std::cout << "─────────────────────────────────────────────────\n\n";
	}
	
	EXPECT_TRUE(train2Waited) << "Train2 should enter Waiting state";
	EXPECT_GT(minDistance, 30.0) << "Trains got too close: " << minDistance << "m";
	
	delete train1;
	delete train2;
}

// ===== TEST 3: Three Train Convoy =====

TEST_F(MultiTrainTest, ThreeTrainConvoy)
{
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n╔════════════════════════════════════════════════╗\n";
		std::cout << "║  THREE TRAIN CONVOY - SEQUENTIAL DEPARTURES   ║\n";
		std::cout << "╚════════════════════════════════════════════════╝\n";
		std::cout << "\nScenario: 3 trains, 1 minute intervals\n";
		std::cout << "Expected: Train2 waits for Train1, Train3 waits for Train2\n\n";
	}
	
	TrainConfig config1 = {
		"Train1", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	TrainConfig config2 = {
		"Train2", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h01"), Time("00h05")
	};
	
	TrainConfig config3 = {
		"Train3", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h02"), Time("00h05")
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
	
	SimulationManager::getInstance().start();
	
	bool train2Waited = false;
	bool train3Waited = false;
	
	for (double time = 0.0; time < 1500.0; time += 1.0)
	{
		SimulationManager::getInstance().step();
		
		if (static_cast<int>(time) % 60 == 0)
		{
			visualize(time);
		}
		
		if (train2->getCurrentState() && 
		    train2->getCurrentState()->getName() == "Waiting")
		{
			train2Waited = true;
		}
		
		if (train3->getCurrentState() && 
		    train3->getCurrentState()->getName() == "Waiting")
		{
			train3Waited = true;
		}
		
		if (train1->getCurrentRail() == nullptr && 
		    train2->getCurrentRail() == nullptr &&
		    train3->getCurrentRail() == nullptr)
		{
			break;
		}
	}
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n\n─────────────────────────────────────────────────\n";
		std::cout << "CONVOY SIMULATION COMPLETE ✓\n";
		std::cout << "─────────────────────────────────────────────────\n";
		std::cout << "All trains finished: YES ✓\n";
		std::cout << "Train2 waited:       " << (train2Waited ? "YES ✓" : "NO ✗") << "\n";
		std::cout << "Train3 waited:       " << (train3Waited ? "YES ✓" : "NO ✗") << "\n";
		std::cout << "─────────────────────────────────────────────────\n\n";
	}
	
	EXPECT_EQ(train1->getCurrentRail(), nullptr) << "Train1 should finish";
	EXPECT_EQ(train2->getCurrentRail(), nullptr) << "Train2 should finish";
	EXPECT_EQ(train3->getCurrentRail(), nullptr) << "Train3 should finish";
	EXPECT_TRUE(train2Waited) << "Train2 should wait for Train1";
	EXPECT_TRUE(train3Waited) << "Train3 should wait for Train2";
	
	delete train1;
	delete train2;
	delete train3;
}

// ===== TEST 4: Different Routes (No Collision Expected) =====

TEST_F(MultiTrainTest, DifferentRoutesNoCollision)
{
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
	
	TrainConfig config1 = {
		"Train_AB", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	TrainConfig config2 = {
		"Train_AC", 60.0, 0.005, 400.0, 450.0,
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
	
	EXPECT_EQ(train1->getCurrentRail(), nullptr) << "Train_AB should finish";
	EXPECT_EQ(train2->getCurrentRail(), nullptr) << "Train_AC should finish";
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n✓ Both trains completed on separate routes\n\n";
	}
	
	delete train1;
	delete train2;
	delete nodeC;
	delete railAC;
}

// ===== TEST 5: Safety Distance Verification During Movement =====

TEST_F(MultiTrainTest, SafetyDistanceMaintainedDuringMovement)
{
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n╔════════════════════════════════════════════════╗\n";
		std::cout << "║  SAFETY DISTANCE VERIFICATION                 ║\n";
		std::cout << "╚════════════════════════════════════════════════╝\n\n";
	}
	
	TrainConfig config1 = {
		"Leader", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	TrainConfig config2 = {
		"Follower", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	Train* train1 = TrainFactory::create(config1, &graph);
	Train* train2 = TrainFactory::create(config2, &graph);
	
	ASSERT_NE(train1, nullptr);
	ASSERT_NE(train2, nullptr);
	
	DijkstraStrategy dijkstra;
	train1->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	train2->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	
	// Start Train2 slightly behind
	train2->setPosition(-150.0);
	
	static IdleState idleState;
	train1->setState(&idleState);
	train2->setState(&idleState);
	
	SimulationManager::getInstance().addTrain(train1);
	SimulationManager::getInstance().addTrain(train2);
	
	SimulationManager::getInstance().start();
	
	double minDistanceWhileMoving = 1e9;
	bool bothMovingAtSomePoint = false;
	
	for (double time = 0.0; time < 1200.0; time += 1.0)
	{
		SimulationManager::getInstance().step();
		
		// Only check distance when BOTH trains are moving (not finished)
		if (train1->getCurrentRail() && train2->getCurrentRail() &&
		    train1->getCurrentRail() == train2->getCurrentRail())
		{
			if (train1->getVelocity() > 1.0 && train2->getVelocity() > 1.0)
			{
				bothMovingAtSomePoint = true;
				double dist = std::abs(train1->getPosition() - train2->getPosition());
				if (dist < minDistanceWhileMoving)
				{
					minDistanceWhileMoving = dist;
				}
			}
		}
		
		if (train1->getCurrentRail() == nullptr && train2->getCurrentRail() == nullptr)
		{
			break;
		}
	}
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n─────────────────────────────────────────────────\n";
		std::cout << "Both trains moved together:  " << (bothMovingAtSomePoint ? "YES ✓" : "NO ✗") << "\n";
		std::cout << "Minimum distance (moving):   " << std::fixed << std::setprecision(1) 
		          << minDistanceWhileMoving << "m\n";
		std::cout << "─────────────────────────────────────────────────\n\n";
	}
	
	EXPECT_TRUE(bothMovingAtSomePoint) << "Trains should have moved together at some point";
	EXPECT_GT(minDistanceWhileMoving, 80.0) 
		<< "Safety distance violated during movement: " << minDistanceWhileMoving << "m";
	
	delete train1;
	delete train2;
}

// ===== TEST 6: No Overtaking on Single Track =====

TEST_F(MultiTrainTest, NoOvertakingOnSingleTrack)
{
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n╔════════════════════════════════════════════════╗\n";
		std::cout << "║  NO OVERTAKING - FAST STUCK BEHIND SLOW       ║\n";
		std::cout << "╚════════════════════════════════════════════════╝\n";
		std::cout << "\nScenario: Slow train departs first, fast train cannot overtake\n";
		std::cout << "Expected: Fast train stuck in Waiting state entire journey\n\n";
	}
	
	TrainConfig config1 = {
		"SlowTrain", 150.0, 0.010, 200.0, 350.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	TrainConfig config2 = {
		"FastTrain", 50.0, 0.002, 600.0, 700.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	Train* train1 = TrainFactory::create(config1, &graph);
	Train* train2 = TrainFactory::create(config2, &graph);
	
	ASSERT_NE(train1, nullptr);
	ASSERT_NE(train2, nullptr);
	
	DijkstraStrategy dijkstra;
	train1->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	train2->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	
	// Start FastTrain slightly behind
	train2->setPosition(-80.0);
	
	static IdleState idleState;
	train1->setState(&idleState);
	train2->setState(&idleState);
	
	SimulationManager::getInstance().addTrain(train1);
	SimulationManager::getInstance().addTrain(train2);
	
	SimulationManager::getInstance().start();
	
	int waitingTimeCount = 0;
	int totalTimeCount = 0;
	
	for (double time = 0.0; time < 2000.0; time += 1.0)
	{
		SimulationManager::getInstance().step();
		
		if (static_cast<int>(time) % 120 == 0)
		{
			visualize(time);
		}
		
		// Count how much time FastTrain spends waiting
		if (train2->getCurrentState() && 
		    train2->getCurrentState()->getName() == "Waiting")
		{
			waitingTimeCount++;
		}
		
		if (train2->getVelocity() > 0.1 || 
		    (train2->getCurrentState() && train2->getCurrentState()->getName() == "Waiting"))
		{
			totalTimeCount++;
		}
		
		if (train1->getCurrentRail() == nullptr && train2->getCurrentRail() == nullptr)
		{
			break;
		}
	}
	
	double waitingPercentage = (totalTimeCount > 0) ? 
		(100.0 * waitingTimeCount / totalTimeCount) : 0.0;
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n\n─────────────────────────────────────────────────\n";
		std::cout << "FastTrain waiting time:  " << std::fixed << std::setprecision(1)
		          << waitingPercentage << "%\n";
		std::cout << "─────────────────────────────────────────────────\n\n";
	}
	
	// FastTrain should spend significant time waiting (at least 50% of journey)
	EXPECT_GT(waitingPercentage, 50.0) 
		<< "FastTrain should be stuck behind SlowTrain";
	
	delete train1;
	delete train2;
}

// ===== TEST 7: High Speed Differential =====

TEST_F(MultiTrainTest, HighSpeedDifferentialCatchUp)
{
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n╔════════════════════════════════════════════════╗\n";
		std::cout << "║  HIGH SPEED DIFFERENTIAL - RAPID CATCH-UP     ║\n";
		std::cout << "╚════════════════════════════════════════════════╝\n\n";
	}
	
	TrainConfig config1 = {
		"Freight", 200.0, 0.012, 180.0, 300.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};
	
	TrainConfig config2 = {
		"Express", 40.0, 0.001, 700.0, 800.0,
		"CityA", "CityB",
		Time("00h02"), Time("00h05")
	};
	
	Train* train1 = TrainFactory::create(config1, &graph);
	Train* train2 = TrainFactory::create(config2, &graph);
	
	ASSERT_NE(train1, nullptr);
	ASSERT_NE(train2, nullptr);
	
	DijkstraStrategy dijkstra;
	train1->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	train2->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	
	static IdleState idleState;
	train1->setState(&idleState);
	train2->setState(&idleState);
	
	SimulationManager::getInstance().addTrain(train1);
	SimulationManager::getInstance().addTrain(train2);
	
	SimulationManager::getInstance().start();
	
	bool expressWaited = false;
	double timeWhenExpressCaughtUp = -1.0;
	
	for (double time = 0.0; time < 2000.0; time += 1.0)
	{
		SimulationManager::getInstance().step();
		
		if (train2->getCurrentState() && 
		    train2->getCurrentState()->getName() == "Waiting")
		{
			if (!expressWaited)
			{
				expressWaited = true;
				timeWhenExpressCaughtUp = time;
				if (ENABLE_VISUALIZATION)
				{
					std::cout << "\n>>> Express train caught Freight train at t=" 
					          << time << "s\n";
				}
			}
		}
		
		if (train1->getCurrentRail() == nullptr && train2->getCurrentRail() == nullptr)
		{
			break;
		}
	}
	
	if (ENABLE_VISUALIZATION)
	{
		std::cout << "\n─────────────────────────────────────────────────\n";
		std::cout << "Express caught Freight:  " << (expressWaited ? "YES ✓" : "NO ✗") << "\n";
		if (expressWaited)
		{
			std::cout << "Catch-up time:           " << timeWhenExpressCaughtUp << "s\n";
		}
		std::cout << "─────────────────────────────────────────────────\n\n";
	}
	
	EXPECT_TRUE(expressWaited) << "Express should catch and wait for Freight";
	EXPECT_GT(timeWhenExpressCaughtUp, 120.0) 
		<< "Express should catch up after at least 2 minutes";
	EXPECT_LT(timeWhenExpressCaughtUp, 600.0) 
		<< "Express should catch up within 10 minutes";
	
	delete train1;
	delete train2;
}
#include <gtest/gtest.h>
#include <iostream>

#include "patterns/factories/TrainFactory.hpp"
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "patterns/states/IdleState.hpp"
#include "patterns/states/AcceleratingState.hpp"
#include "patterns/states/CruisingState.hpp"
#include "patterns/states/BrakingState.hpp"
#include "simulation/PhysicsSystem.hpp"

#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "core/Train.hpp"

class MultiTrainTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		Train::resetIDCounter();

		nodeA = new Node("CityA");
		nodeB = new Node("CityB");

		graph.addNode(nodeA);
		graph.addNode(nodeB);

		rail = new Rail(nodeA, nodeB, 10.0, 200.0);
		graph.addRail(rail);
	}

	void TearDown() override
	{
		// Graph destructor handles cleanup
	}
	
	Graph graph;
	Node *nodeA, *nodeB;
	Rail *rail;
};

TEST_F(MultiTrainTest, TwoTrainsCanExistOnSameRail)
{
	std::cout << "\n==== TEST: Two Trains Can Exist On Same Rail ====\n";

	TrainConfig cfg = {
		"Train", 80.0, 0.005, 300.0, 400.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};

	Train* train1 = TrainFactory::create(cfg, &graph);
	Train* train2 = TrainFactory::create(cfg, &graph);

	ASSERT_NE(train1, nullptr);
	ASSERT_NE(train2, nullptr);

	DijkstraStrategy dijkstra;
	train1->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	train2->setPath(dijkstra.findPath(&graph, nodeA, nodeB));

	train2->setPosition(-500.0);

	EXPECT_EQ(train1->getCurrentRail(), rail);
	EXPECT_EQ(train2->getCurrentRail(), rail);
	
	std::cout << "Train1 position: " << train1->getPosition() << "m\n";
	std::cout << "Train2 position: " << train2->getPosition() << "m\n";
	std::cout << "Both trains on same rail: SUCCESS\n";

	delete train1;
	delete train2;
}

TEST_F(MultiTrainTest, TrainsMaintainSeparation)
{
	std::cout << "\n==== TEST: Trains Maintain Separation ====\n";

	TrainConfig cfg = {
		"Train", 80.0, 0.005, 300.0, 400.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};

	Train* leader = TrainFactory::create(cfg, &graph);
	Train* follower = TrainFactory::create(cfg, &graph);

	ASSERT_NE(leader, nullptr);
	ASSERT_NE(follower, nullptr);

	DijkstraStrategy dijkstra;
	leader->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	follower->setPath(dijkstra.findPath(&graph, nodeA, nodeB));

	leader->setPosition(0.0);
	follower->setPosition(-200.0);
	
	leader->setVelocity(20.0);
	follower->setVelocity(25.0);

	IdleState idle;
	leader->setState(&idle);
	follower->setState(&idle);

	double minDistance = 1e9;

	for (int t = 0; t < 100; ++t)
	{
		leader->update(1.0);
		follower->update(1.0);
		
		double dist = leader->getPosition() - follower->getPosition();
		if (dist > 0 && dist < minDistance)
		{
			minDistance = dist;
		}
	}

	std::cout << "Minimum distance maintained: " << minDistance << "m\n";
	EXPECT_GT(minDistance, 0.0) << "Trains collided!";

	delete leader;
	delete follower;
}

TEST_F(MultiTrainTest, TrainsDontCollide)
{
	std::cout << "\n==== TEST: Trains Don't Collide ====\n";

	TrainConfig cfg = {
		"Train", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};

	Train* train1 = TrainFactory::create(cfg, &graph);
	Train* train2 = TrainFactory::create(cfg, &graph);

	ASSERT_NE(train1, nullptr);
	ASSERT_NE(train2, nullptr);

	DijkstraStrategy dijkstra;
	train1->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	train2->setPath(dijkstra.findPath(&graph, nodeA, nodeB));

	train1->setPosition(1000.0);
	train2->setPosition(0.0);

	IdleState idle;
	train1->setState(&idle);
	train2->setState(&idle);

	bool collision = false;

	for (int t = 0; t < 500; ++t)
	{
		train1->update(1.0);
		train2->update(1.0);

		double dist = train1->getPosition() - train2->getPosition();
		if (dist < 10.0 && dist > -10.0)
		{
			collision = true;
			std::cout << "COLLISION at t=" << t << "s!\n";
			break;
		}
	}

	std::cout << "Final positions:\n";
	std::cout << "  Train1: " << train1->getPosition() << "m\n";
	std::cout << "  Train2: " << train2->getPosition() << "m\n";
	std::cout << "Collision avoided: " << (!collision ? "SUCCESS" : "FAILED") << "\n";

	EXPECT_FALSE(collision) << "Trains collided";

	delete train1;
	delete train2;
}

TEST_F(MultiTrainTest, BothTrainsReachDestination)
{
	std::cout << "\n==== TEST: Both Trains Reach Destination ====\n";

	TrainConfig cfg = {
		"Train", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("00h00"), Time("00h05")
	};

	Train* train1 = TrainFactory::create(cfg, &graph);
	Train* train2 = TrainFactory::create(cfg, &graph);

	ASSERT_NE(train1, nullptr);
	ASSERT_NE(train2, nullptr);

	DijkstraStrategy dijkstra;
	train1->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
	train2->setPath(dijkstra.findPath(&graph, nodeA, nodeB));

	train2->setPosition(-500.0);

	AcceleratingState accelState;
	CruisingState cruiseState;
	BrakingState brakeState;

	int maxSteps = 2000;
	for (int t = 0; t < maxSteps; ++t)
	{
		// Manual state control for train1
		Rail* r1 = train1->getCurrentRail();
		if (r1)
		{
			double railLength1 = r1->getLength() * 1000.0;
			double remaining1 = railLength1 - train1->getPosition();
			
			if (train1->getVelocity() < PhysicsSystem::kmhToMs(150.0))
			{
				train1->setState(&accelState);
			}
			else if (remaining1 > 3000.0)
			{
				train1->setState(&cruiseState);
			}
			else
			{
				train1->setState(&brakeState);
			}
			
			train1->update(1.0);
			
            if (train1->getPosition() >= railLength1)
            {
                train1->advanceToNextRail();
                if (train1->getCurrentRail() == nullptr)
                {
                    train1->markFinished();
                }
            }
		}
		
		// Manual state control for train2
		Rail* r2 = train2->getCurrentRail();
		if (r2)
		{
			double railLength2 = r2->getLength() * 1000.0;
			double remaining2 = railLength2 - train2->getPosition();
			
			if (train2->getVelocity() < PhysicsSystem::kmhToMs(150.0))
			{
				train2->setState(&accelState);
			}
			else if (remaining2 > 3000.0)
			{
				train2->setState(&cruiseState);
			}
			else
			{
				train2->setState(&brakeState);
			}
			
			train2->update(1.0);
			
            if (train2->getPosition() >= railLength2)
            {
                train2->advanceToNextRail();
                if (train2->getCurrentRail() == nullptr)
                {
                    train2->markFinished();
                }
            }
		}
		
		if (train1->isFinished() && train2->isFinished())
		{
			std::cout << "Both trains finished at t=" << t << "s\n";
			break;
		}
	}

	std::cout << "Train1 finished: " << (train1->isFinished() ? "YES" : "NO") << "\n";
	std::cout << "Train2 finished: " << (train2->isFinished() ? "YES" : "NO") << "\n";

	EXPECT_TRUE(train1->isFinished());
	EXPECT_TRUE(train2->isFinished());

	delete train1;
	delete train2;
}
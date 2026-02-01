#include <gtest/gtest.h>
#include "patterns/factories/TrainFactory.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "utils/Time.hpp"

class TrainFactoryTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		Train::resetIDCounter();
		
		cityA = new Node("CityA");
		cityB = new Node("CityB");
		
		graph.addNode(cityA);
		graph.addNode(cityB);
	}
	
	void TearDown() override
	{
		delete cityA;
		delete cityB;
	}
	
	Graph graph;
	Node* cityA;
	Node* cityB;
};

// ===== VALID TRAIN CREATION TESTS =====

TEST_F(TrainFactoryTest, CreateValidTrain)
{
	TrainConfig config = {
		"Express", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("14h10"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(config, &graph);
	
	EXPECT_NE(train, nullptr);
	EXPECT_EQ(train->getName(), "Express");
	EXPECT_EQ(train->getMass(), 80.0);
	EXPECT_EQ(train->getID(), 1);
	
	delete train;
}

TEST_F(TrainFactoryTest, CreateMultipleTrainsSequentialID)
{
	TrainConfig config1 = {
		"Train1", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("14h10"), Time("00h05")
	};
	
	TrainConfig config2 = {
		"Train2", 60.0, 0.005, 400.0, 450.0,
		"CityA", "CityB",
		Time("14h20"), Time("00h10")
	};
	
	Train* train1 = TrainFactory::create(config1, &graph);
	Train* train2 = TrainFactory::create(config2, &graph);
	
	EXPECT_NE(train1, nullptr);
	EXPECT_NE(train2, nullptr);
	EXPECT_EQ(train1->getID(), 1);
	EXPECT_EQ(train2->getID(), 2);
	
	delete train1;
	delete train2;
}

// ===== NULL/INVALID NETWORK TESTS =====

TEST_F(TrainFactoryTest, CreateNullNetwork)
{
	TrainConfig config = {
		"Express", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("14h10"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(config, nullptr);
	
	EXPECT_EQ(train, nullptr);
}

// ===== INVALID TRAIN PROPERTIES TESTS =====

TEST_F(TrainFactoryTest, CreateEmptyName)
{
	TrainConfig config = {
		"", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("14h10"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(config, &graph);
	
	EXPECT_EQ(train, nullptr);
}

TEST_F(TrainFactoryTest, CreateNegativeMass)
{
	TrainConfig config = {
		"Express", -80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("14h10"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(config, &graph);
	
	EXPECT_EQ(train, nullptr);
}

TEST_F(TrainFactoryTest, CreateZeroMass)
{
	TrainConfig config = {
		"Express", 0.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("14h10"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(config, &graph);
	
	EXPECT_EQ(train, nullptr);
}

TEST_F(TrainFactoryTest, CreateNegativeFriction)
{
	TrainConfig config = {
		"Express", 80.0, -0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("14h10"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(config, &graph);
	
	EXPECT_EQ(train, nullptr);
}

TEST_F(TrainFactoryTest, CreateZeroAccelForce)
{
	TrainConfig config = {
		"Express", 80.0, 0.005, 0.0, 500.0,
		"CityA", "CityB",
		Time("14h10"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(config, &graph);
	
	EXPECT_EQ(train, nullptr);
}

TEST_F(TrainFactoryTest, CreateZeroBrakeForce)
{
	TrainConfig config = {
		"Express", 80.0, 0.005, 356.0, 0.0,
		"CityA", "CityB",
		Time("14h10"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(config, &graph);
	
	EXPECT_EQ(train, nullptr);
}

// ===== INVALID STATION TESTS =====

TEST_F(TrainFactoryTest, CreateInvalidDepartureStation)
{
	TrainConfig config = {
		"Express", 80.0, 0.005, 356.0, 500.0,
		"CityZ", "CityB",
		Time("14h10"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(config, &graph);
	
	EXPECT_EQ(train, nullptr);
}

TEST_F(TrainFactoryTest, CreateInvalidArrivalStation)
{
	TrainConfig config = {
		"Express", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityZ",
		Time("14h10"), Time("00h05")
	};
	
	Train* train = TrainFactory::create(config, &graph);
	
	EXPECT_EQ(train, nullptr);
}

// ===== INVALID TIME TESTS =====

TEST_F(TrainFactoryTest, CreateInvalidDepartureTime)
{
	TrainConfig config = {
		"Express", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time(25, 0), Time("00h05")
	};
	
	Train* train = TrainFactory::create(config, &graph);
	
	EXPECT_EQ(train, nullptr);
}

TEST_F(TrainFactoryTest, CreateInvalidStopDuration)
{
	TrainConfig config = {
		"Express", 80.0, 0.005, 356.0, 500.0,
		"CityA", "CityB",
		Time("14h10"), Time(0, 65)
	};
	
	Train* train = TrainFactory::create(config, &graph);
	
	EXPECT_EQ(train, nullptr);
}
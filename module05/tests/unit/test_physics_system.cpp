#include <gtest/gtest.h>
#include "simulation/PhysicsSystem.hpp"
#include "core/Train.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "utils/Time.hpp"
#include <cmath>

class PhysicsSystemTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		Train::resetIDCounter();
		
		nodeA = new Node("CityA");
		nodeB = new Node("CityB");
		
		// Standard test train: 80 tons, friction 0.005, 356kN accel, 500kN brake
		Time depTime("10h00");
		Time stopDur("00h05");
		train = new Train("TestTrain", 80.0, 0.005, 356.0, 500.0,
		                  "CityA", "CityB", depTime, stopDur);
		
		// Standard rail: 50km, 250 km/h limit
		rail = new Rail(nodeA, nodeB, 50.0, 250.0);
	}

	void TearDown() override
	{
		delete train;
		delete rail;
		delete nodeA;
		delete nodeB;
	}

	Train* train;
	Rail* rail;
	Node* nodeA;
	Node* nodeB;
};

// ===== UNIT CONVERSION TESTS =====

TEST_F(PhysicsSystemTest, TonsToKg)
{
	EXPECT_DOUBLE_EQ(PhysicsSystem::tonsToKg(80.0), 80000.0);
	EXPECT_DOUBLE_EQ(PhysicsSystem::tonsToKg(1.0), 1000.0);
	EXPECT_DOUBLE_EQ(PhysicsSystem::tonsToKg(0.0), 0.0);
}

TEST_F(PhysicsSystemTest, KNtoN)
{
	EXPECT_DOUBLE_EQ(PhysicsSystem::kNtoN(356.0), 356000.0);
	EXPECT_DOUBLE_EQ(PhysicsSystem::kNtoN(1.0), 1000.0);
	EXPECT_DOUBLE_EQ(PhysicsSystem::kNtoN(0.0), 0.0);
}

TEST_F(PhysicsSystemTest, KmhToMs)
{
	EXPECT_NEAR(PhysicsSystem::kmhToMs(250.0), 69.44, 0.01);
	EXPECT_NEAR(PhysicsSystem::kmhToMs(100.0), 27.78, 0.01);
	EXPECT_DOUBLE_EQ(PhysicsSystem::kmhToMs(0.0), 0.0);
}

TEST_F(PhysicsSystemTest, KmToM)
{
	EXPECT_DOUBLE_EQ(PhysicsSystem::kmToM(50.0), 50000.0);
	EXPECT_DOUBLE_EQ(PhysicsSystem::kmToM(1.0), 1000.0);
	EXPECT_DOUBLE_EQ(PhysicsSystem::kmToM(0.0), 0.0);
}

TEST_F(PhysicsSystemTest, MsToKmh)
{
	EXPECT_NEAR(PhysicsSystem::msToKmh(69.44), 250.0, 0.2);
	EXPECT_NEAR(PhysicsSystem::msToKmh(27.78), 100.0, 0.1);
	EXPECT_DOUBLE_EQ(PhysicsSystem::msToKmh(0.0), 0.0);
}

// ===== FORCE CALCULATION TESTS =====

TEST_F(PhysicsSystemTest, CalculateFriction)
{
	// F_friction = μ * m * g
	// = 0.005 * 80000 * 9.8 = 3920 N
	double friction = PhysicsSystem::calculateFriction(train);
	EXPECT_NEAR(friction, 3920.0, 0.1);
}

TEST_F(PhysicsSystemTest, CalculateFrictionNullTrain)
{
	double friction = PhysicsSystem::calculateFriction(nullptr);
	EXPECT_DOUBLE_EQ(friction, 0.0);
}

TEST_F(PhysicsSystemTest, CalculateNetForce)
{
	// Applied: 356000 N, Friction: 3920 N
	// Net = 356000 - 3920 = 352080 N
	double accelForceN = PhysicsSystem::kNtoN(356.0);
	double netForce = PhysicsSystem::calculateNetForce(train, accelForceN);
	EXPECT_NEAR(netForce, 352080.0, 1.0);
}

TEST_F(PhysicsSystemTest, CalculateNetForceZeroApplied)
{
	double netForce = PhysicsSystem::calculateNetForce(train, 0.0);
	EXPECT_NEAR(netForce, -3920.0, 0.1);
}

TEST_F(PhysicsSystemTest, CalculateBrakingDeceleration)
{
	// (500000 + 3920) / 80000 = 6.299 m/s²
	double decel = PhysicsSystem::calculateBrakingDeceleration(train);
	EXPECT_NEAR(decel, 6.299, 0.01);
}

TEST_F(PhysicsSystemTest, CalculateBrakingDistance)
{
	// At 250 km/h = 69.44 m/s
	// d = v² / (2*a) = 69.44² / (2*6.299) = 383 meters
	train->setVelocity(69.44);
	double distance = PhysicsSystem::calculateBrakingDistance(train);
	EXPECT_NEAR(distance, 383.0, 5.0);
}

TEST_F(PhysicsSystemTest, BrakingDistanceAtRest)
{
	train->setVelocity(0.0);
	double distance = PhysicsSystem::calculateBrakingDistance(train);
	EXPECT_DOUBLE_EQ(distance, 0.0);
}

TEST_F(PhysicsSystemTest, BrakingDistanceAtLowSpeed)
{
	train->setVelocity(10.0);  // 10 m/s = 36 km/h
	double distance = PhysicsSystem::calculateBrakingDistance(train);
	EXPECT_NEAR(distance, 7.94, 0.1);
}

// ===== MOTION UPDATE TESTS =====

TEST_F(PhysicsSystemTest, UpdateVelocityAcceleration)
{
	train->setVelocity(0.0);
	
	// Net force = 352080 N, mass = 80000 kg
	// a = 4.401 m/s², dt = 1s
	// v_new = 0 + 4.401 = 4.401 m/s
	double accelForceN = PhysicsSystem::kNtoN(356.0);
	double netForce = PhysicsSystem::calculateNetForce(train, accelForceN);
	
	PhysicsSystem::updateVelocity(train, netForce, 1.0);
	
	EXPECT_NEAR(train->getVelocity(), 4.401, 0.01);
}

TEST_F(PhysicsSystemTest, UpdateVelocityMultipleTimesteps)
{
	train->setVelocity(0.0);
	
	double accelForceN = PhysicsSystem::kNtoN(356.0);
	double netForce = PhysicsSystem::calculateNetForce(train, accelForceN);
	
	for (int i = 0; i < 10; i++)
	{
		PhysicsSystem::updateVelocity(train, netForce, 1.0);
	}
	
	EXPECT_NEAR(train->getVelocity(), 44.01, 0.1);
}

TEST_F(PhysicsSystemTest, UpdateVelocityNoNegative)
{
	train->setVelocity(5.0);
	
	// Apply huge negative force
	PhysicsSystem::updateVelocity(train, -1000000.0, 1.0);
	
	EXPECT_DOUBLE_EQ(train->getVelocity(), 0.0);
}

TEST_F(PhysicsSystemTest, UpdatePosition)
{
	train->setPosition(0.0);
	train->setVelocity(20.0);  // 20 m/s
	
	PhysicsSystem::updatePosition(train, 1.0);
	
	EXPECT_DOUBLE_EQ(train->getPosition(), 20.0);
}

TEST_F(PhysicsSystemTest, UpdatePositionMultipleTimesteps)
{
	train->setPosition(0.0);
	train->setVelocity(10.0);
	
	for (int i = 0; i < 5; i++)
	{
		PhysicsSystem::updatePosition(train, 1.0);
	}
	
	EXPECT_DOUBLE_EQ(train->getPosition(), 50.0);
}

TEST_F(PhysicsSystemTest, UpdatePositionWithAcceleration)
{
	train->setPosition(0.0);
	train->setVelocity(0.0);
	
	double accelForceN = PhysicsSystem::kNtoN(356.0);
	double netForce = PhysicsSystem::calculateNetForce(train, accelForceN);
	
	// First timestep: v becomes 4.401, then position updates
	PhysicsSystem::updateVelocity(train, netForce, 1.0);
	PhysicsSystem::updatePosition(train, 1.0);
	EXPECT_NEAR(train->getPosition(), 4.401, 0.01);
	
	// Second timestep: v becomes 8.802, position += 8.802
	PhysicsSystem::updateVelocity(train, netForce, 1.0);
	PhysicsSystem::updatePosition(train, 1.0);
	EXPECT_NEAR(train->getPosition(), 13.203, 0.01);
}

// ===== NULL SAFETY TESTS =====

TEST_F(PhysicsSystemTest, UpdateVelocityNullTrain)
{
	PhysicsSystem::updateVelocity(nullptr, 1000.0, 1.0);
	// Should not crash
}

TEST_F(PhysicsSystemTest, UpdatePositionNullTrain)
{
	PhysicsSystem::updatePosition(nullptr, 1.0);
	// Should not crash
}
#include <gtest/gtest.h>
#include "patterns/states/IdleState.hpp"
#include "patterns/states/AcceleratingState.hpp"
#include "patterns/states/StoppedState.hpp"
#include "core/Train.hpp"
#include "utils/Time.hpp"

class TrainStateTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		Train::resetIDCounter();
		
		Time depTime("10h00");
		Time stopDur("00h05");
		train = new Train("TestTrain", 80.0, 0.005, 356.0, 500.0,
		                  "CityA", "CityB", depTime, stopDur);
	}
	
	void TearDown() override
	{
		delete train;
	}
	
	Train* train;
};

// ===== IDLE STATE TESTS =====

TEST_F(TrainStateTest, IdleStateKeepsVelocityZero)
{
	IdleState idleState;
	train->setState(&idleState);
	
	train->setVelocity(10.0);
	train->update(1.0);
	
	EXPECT_EQ(train->getVelocity(), 0.0);
	EXPECT_EQ(idleState.getName(), "Idle");
}

TEST_F(TrainStateTest, IdleStateMultipleUpdates)
{
	IdleState idleState;
	train->setState(&idleState);
	
	train->update(1.0);
	train->update(1.0);
	train->update(1.0);
	
	EXPECT_EQ(train->getVelocity(), 0.0);
}

// ===== ACCELERATING STATE TESTS =====

TEST_F(TrainStateTest, AcceleratingStateRequiresRail)
{
	AcceleratingState accelState;
	train->setState(&accelState);
	
	// No rail set - should not crash, velocity stays 0
	train->update(1.0);
	EXPECT_EQ(train->getVelocity(), 0.0);
	
	EXPECT_EQ(accelState.getName(), "Accelerating");
}

// ===== STOPPED STATE TESTS =====

TEST_F(TrainStateTest, StoppedStateKeepsVelocityZero)
{
	StoppedState stoppedState(10.0);
	train->setState(&stoppedState);
	
	train->setVelocity(50.0);
	train->update(1.0);
	
	EXPECT_EQ(train->getVelocity(), 0.0);
	EXPECT_EQ(stoppedState.getName(), "Stopped");
}

TEST_F(TrainStateTest, StoppedStateCountsDown)
{
	StoppedState stoppedState(10.0);
	train->setState(&stoppedState);
	
	EXPECT_EQ(stoppedState.getTimeRemaining(), 10.0);
	
	train->update(1.0);
	EXPECT_EQ(stoppedState.getTimeRemaining(), 9.0);
	
	train->update(3.0);
	EXPECT_EQ(stoppedState.getTimeRemaining(), 6.0);
}

TEST_F(TrainStateTest, StoppedStateReachesZero)
{
	StoppedState stoppedState(5.0);
	train->setState(&stoppedState);
	
	train->update(3.0);
	EXPECT_EQ(stoppedState.getTimeRemaining(), 2.0);
	
	train->update(3.0);
	EXPECT_EQ(stoppedState.getTimeRemaining(), 0.0);
}

TEST_F(TrainStateTest, StoppedStateDoesNotGoNegative)
{
	StoppedState stoppedState(2.0);
	train->setState(&stoppedState);
	
	train->update(5.0);
	EXPECT_EQ(stoppedState.getTimeRemaining(), 0.0);
}

// ===== STATE TRANSITION TESTS =====

TEST_F(TrainStateTest, StateTransitionIdleToAccelerating)
{
	IdleState idleState;
	AcceleratingState accelState;
	
	train->setState(&idleState);
	train->update(1.0);
	EXPECT_EQ(train->getVelocity(), 0.0);
	
	// Transition to accelerating (no rail, so still 0)
	train->setState(&accelState);
	train->update(1.0);
	EXPECT_EQ(train->getVelocity(), 0.0);
}

TEST_F(TrainStateTest, StateTransitionAcceleratingToStopped)
{
	AcceleratingState accelState;
	StoppedState stoppedState(5.0);
	
	train->setVelocity(20.0);  // Set manually for this test
	
	train->setState(&stoppedState);
	train->update(1.0);
	EXPECT_EQ(train->getVelocity(), 0.0);
	EXPECT_EQ(stoppedState.getTimeRemaining(), 4.0);
}

// ===== NULL SAFETY TESTS =====

TEST_F(TrainStateTest, NullStateDoesNotCrash)
{
	train->setState(nullptr);
	train->update(1.0);
	
	// Should not crash, velocity unchanged
	EXPECT_EQ(train->getVelocity(), 0.0);
}

TEST_F(TrainStateTest, GetCurrentState)
{
	IdleState idleState;
	train->setState(&idleState);
	
	EXPECT_EQ(train->getCurrentState(), &idleState);
}
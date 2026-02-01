#include "patterns/states/AcceleratingState.hpp"
#include "patterns/states/CruisingState.hpp"
#include "patterns/states/WaitingState.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "simulation/SimulationContext.hpp"

void AcceleratingState::update(Train* train, double dt)
{
	if (!train)
	{
		return;
	}
	
	// Get current rail to check speed limit
	Rail* currentRail = train->getCurrentRail();
	if (!currentRail)
	{
		// No rail, cannot accelerate
		return;
	}
	
	// Apply maximum acceleration force
	double accelForceN = PhysicsSystem::kNtoN(train->getMaxAccelForce());
	double netForce = PhysicsSystem::calculateNetForce(train, accelForceN);
	
	PhysicsSystem::updateVelocity(train, netForce, dt);
	
	// Enforce speed limit
	double speedLimitMs = PhysicsSystem::kmhToMs(currentRail->getSpeedLimit());
	if (train->getVelocity() > speedLimitMs)
	{
		train->setVelocity(speedLimitMs);
	}
	
	// Update position
	PhysicsSystem::updatePosition(train, dt);
}

ITrainState* AcceleratingState::checkTransition(Train* train, SimulationContext* ctx)
{
	if (!train || !ctx)
	{
		return nullptr;
	}

	if (ctx->shouldWaitForTrainAhead(train))
	{
		static WaitingState waitingState;
		return &waitingState;
	}

	double speedLimitMs = ctx->getCurrentRailSpeedLimit(train);
	if (train->getVelocity() >= speedLimitMs * 0.99)
	{
		static CruisingState cruisingState;
		return &cruisingState;
	}

	return nullptr;
}


std::string AcceleratingState::getName() const
{
	return "Accelerating";
}
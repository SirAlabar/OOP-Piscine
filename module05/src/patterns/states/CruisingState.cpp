#include "patterns/states/CruisingState.hpp"
#include "patterns/states/WaitingState.hpp"
#include "patterns/states/BrakingState.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "simulation/SimulationContext.hpp"

void CruisingState::update(Train* train, double dt)
{
	if (!train)
	{
		return;
	}
	
	// Get current rail to check speed limit
	Rail* currentRail = train->getCurrentRail();
	if (!currentRail)
	{
		// No rail, stay at current velocity
		return;
	}
	
	// Convert speed limit to m/s
	double speedLimitMs = PhysicsSystem::kmhToMs(currentRail->getSpeedLimit());
	
	// Maintain speed at or below limit
	double currentVelocity = train->getVelocity();
	
	if (currentVelocity > speedLimitMs)
	{
		// Exceeding limit - apply light braking (friction only)
		double frictionForce = PhysicsSystem::calculateFriction(train);
		double netForce = -frictionForce;
		
		PhysicsSystem::updateVelocity(train, netForce, dt);
	}
	else if (currentVelocity < speedLimitMs)
	{
		// Below limit - apply traction to reach it
		double accelForceN = PhysicsSystem::kNtoN(train->getMaxAccelForce());
		double netForce = PhysicsSystem::calculateNetForce(train, accelForceN);
		
		PhysicsSystem::updateVelocity(train, netForce, dt);
		
		// Cap at speed limit
		if (train->getVelocity() > speedLimitMs)
		{
			train->setVelocity(speedLimitMs);
		}
	}
	// else: exactly at limit, maintain (friction balanced by traction)
	
	// Update position
	PhysicsSystem::updatePosition(train, dt);
}

ITrainState* CruisingState::checkTransition(Train* train, SimulationContext* ctx)
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

	double brakingDist = ctx->getBrakingDistance(train);
	double distRemaining = ctx->getDistanceToRailEnd(train);

	if (distRemaining <= brakingDist * 1.1)
	{
		static BrakingState brakingState;
		return &brakingState;
	}

	return nullptr;
}


std::string CruisingState::getName() const
{
	return "Cruising";
}
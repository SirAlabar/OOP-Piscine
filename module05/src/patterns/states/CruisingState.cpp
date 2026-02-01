#include "patterns/states/CruisingState.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "simulation/PhysicsSystem.hpp"

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

std::string CruisingState::getName() const
{
	return "Cruising";
}
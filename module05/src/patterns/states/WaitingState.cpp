#include "patterns/states/WaitingState.hpp"
#include "patterns/states/CruisingState.hpp"
#include "patterns/states/AcceleratingState.hpp"
#include "core/Train.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "simulation/SimulationContext.hpp"

void WaitingState::update(Train* train, double dt)
{
	if (!train)
	{
		return;
	}
	
	// Gentle deceleration (friction only, no active braking)
	// Train slows naturally when blocked
	double frictionForce = PhysicsSystem::calculateFriction(train);
	double netForce = -frictionForce;
	
	PhysicsSystem::updateVelocity(train, netForce, dt);
	
	// Update position
	PhysicsSystem::updatePosition(train, dt);
	
	// If velocity reaches 0, stop
	if (train->getVelocity() <= 0.0)
	{
		train->setVelocity(0.0);
	}
}

ITrainState* WaitingState::checkTransition(Train* train, SimulationContext* ctx)
{
	if (!train || !ctx)
	{
		return nullptr;
	}
	
	// Transition: Waiting → Cruising/Accelerating (rail cleared)
	if (ctx->isNextTrainTooClose(train))
	{
		double speedLimit = ctx->getCurrentRailSpeedLimit(train);
		
		// Resume at cruising if already at speed
		if (train->getVelocity() >= speedLimit * 0.9)
		{
			static CruisingState cruisingState;
			return &cruisingState;
		}
		// Otherwise accelerate
		else
		{
			static AcceleratingState accelState;
			return &accelState;
		}
	}
	
	return nullptr;
}

std::string WaitingState::getName() const
{
	return "Waiting";
}
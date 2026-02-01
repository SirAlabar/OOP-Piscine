#include "patterns/states/WaitingState.hpp"
#include "core/Train.hpp"
#include "simulation/PhysicsSystem.hpp"

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

std::string WaitingState::getName() const
{
	return "Waiting";
}
#include "patterns/states/BrakingState.hpp"
#include "core/Train.hpp"
#include "simulation/PhysicsSystem.hpp"

void BrakingState::update(Train* train, double dt)
{
	if (!train)
	{
		return;
	}
	
	// Apply maximum braking force
	double brakeForceN = PhysicsSystem::kNtoN(train->getMaxBrakeForce());
	double frictionForce = PhysicsSystem::calculateFriction(train);
	
	// Both oppose motion (negative force)
	double netForce = -(brakeForceN + frictionForce);
	
	PhysicsSystem::updateVelocity(train, netForce, dt);
	
	// Update position
	PhysicsSystem::updatePosition(train, dt);
	
	// If velocity reaches 0, stop exactly
	if (train->getVelocity() <= 0.0)
	{
		train->setVelocity(0.0);
	}
}

std::string BrakingState::getName() const
{
	return "Braking";
}
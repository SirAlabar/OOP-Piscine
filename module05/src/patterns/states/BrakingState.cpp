#include "patterns/states/BrakingState.hpp"
#include "patterns/states/StoppedState.hpp"
#include "patterns/states/AcceleratingState.hpp"
#include "core/Train.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "simulation/SimulationContext.hpp"

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

ITrainState* BrakingState::checkTransition(Train* train, SimulationContext* ctx)
{
	if (!train || !ctx)
	{
		return nullptr;
	}
	
	double railLength = ctx->getCurrentRailLength(train);
	
	// Transition: Braking → Stopped (arrived at rail end)
	if (train->getVelocity() <= 0.01 && 
	    train->getPosition() >= railLength * 0.99)
	{
		train->setVelocity(0.0);
		train->setPosition(railLength);
		
		// Clear rail occupancy
		Rail* currentRail = train->getCurrentRail();
		if (currentRail)
		{
			currentRail->clearOccupied();
		}
		
		// Get arrival node BEFORE advancing
		Node* arrivalNode = ctx->getCurrentArrivalNode(train);
		
		// Advance to next rail
		train->advanceToNextRail();
		
		if (train->getCurrentRail())
		{
			train->setPosition(0.0);
			
			// Station (City) → Stop with duration
			if (arrivalNode && arrivalNode->getType() == NodeType::CITY)
			{
				double stopSeconds = train->getStopDuration().toMinutes() * 60.0;
				return ctx->getOrCreateStoppedState(train, stopSeconds);
			}
			// Junction → Immediately accelerate
			else
			{
				static AcceleratingState accelState;
				return &accelState;
			}
		}
		// No more rails - journey complete (return nullptr, train will handle)
	}
	
	return nullptr;
}

std::string BrakingState::getName() const
{
	return "Braking";
}
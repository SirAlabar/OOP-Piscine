#include "patterns/states/BrakingState.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "core/Train.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/PhysicsSystem.hpp"

void BrakingState::update(Train* train, double dt)
{
	if (!train)
	{
		return;
	}
	
	double brakeForceN = PhysicsSystem::kNtoN(train->getMaxBrakeForce());
	double frictionForce = PhysicsSystem::calculateFriction(train);
	
	double netForce = -(brakeForceN + frictionForce);
	
	PhysicsSystem::updateVelocity(train, netForce, dt);
	PhysicsSystem::updatePosition(train, dt);
	
	if (train->getVelocity() <= 0.01)
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
	
	if (train->getVelocity() <= 0.01 && train->getPosition() >= railLength * 0.99)
	{
		train->setVelocity(0.0);
		train->setPosition(railLength);
		
		Rail* currentRail = train->getCurrentRail();
		if (currentRail)
		{
			currentRail->clearOccupied();
		}
		
		Node* arrivalNode = ctx->getCurrentArrivalNode(train);
		
		train->advanceToNextRail();
		
		if (!train->getCurrentRail())
		{
			return nullptr;
		}
		
		train->setPosition(0.0);
		
		if (arrivalNode && arrivalNode->getType() == NodeType::CITY)
		{
			double stopSeconds = train->getStopDuration().toMinutes() * 60.0;
			ctx->setStopDuration(train, stopSeconds);
			return ctx->states().stopped();
		}
		
		return ctx->states().accelerating();
	}
	
	return nullptr;
}

std::string BrakingState::getName() const
{
	return "Braking";
}
#include "patterns/states/WaitingState.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "core/Train.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "simulation/SafetyConstants.hpp"
#include "simulation/RiskData.hpp"
#include <algorithm>

void WaitingState::update(Train* train, double dt)
{
	if (!train)
	{
		return;
	}
	
	double frictionForce = PhysicsSystem::calculateFriction(train);
	double netForce = -frictionForce;
	PhysicsSystem::updateVelocity(train, netForce, dt);
	PhysicsSystem::updatePosition(train, dt);
	
	if (train->getVelocity() < 0.0)
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
	
	const RiskData& risk = ctx->getRisk(train);
	
	if (SafetyConstants::isEmergencyZone(risk.gap, risk.brakingDistance))
	{
		return ctx->states().emergency();
	}
	
	if (SafetyConstants::isFreeZone(risk.gap, risk.safeDistance))
	{
		double speedLimit = ctx->getCurrentRailSpeedLimit(train);
		
		if (train->getVelocity() >= speedLimit * 0.9)
		{
			return ctx->states().cruising();
		}
		
		return ctx->states().accelerating();
	}
	
	return nullptr;
}

std::string WaitingState::getName() const
{
	return "Waiting";
}
#include "patterns/states/CruisingState.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "simulation/SafetyConstants.hpp"
#include "simulation/RiskData.hpp"

void CruisingState::update(Train* train, double dt)
{
	if (!train)
	{
		return;
	}
	
	Rail* currentRail = train->getCurrentRail();
	if (!currentRail)
	{
		return;
	}
	
	double speedLimitMs = PhysicsSystem::kmhToMs(currentRail->getSpeedLimit());
	double currentVelocity = train->getVelocity();
	
	if (currentVelocity > speedLimitMs)
	{
		// Exceeding limit - apply light braking (friction only)
		double frictionForce = PhysicsSystem::calculateFriction(train);
		double netForce = -frictionForce;
		
		PhysicsSystem::updateVelocity(train, netForce, dt);
	}
	else if (currentVelocity < speedLimitMs * 0.95)
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
	
	PhysicsSystem::updatePosition(train, dt);
}

ITrainState* CruisingState::checkTransition(Train* train, SimulationContext* ctx)
{
	if (!train || !ctx)
	{
		return nullptr;
	}
	
	const RiskData& risk = ctx->getRisk(train);
	
	// Priority 1: Emergency zone
	if (SafetyConstants::isEmergencyZone(risk.gap, risk.brakingDistance))
	{
		return ctx->states().emergency();
	}
	
	// Priority 2: Following zone
	if (SafetyConstants::isFollowingZone(risk.gap, risk.brakingDistance, risk.safeDistance))
	{
		return ctx->states().waiting();
	}
	
	// Priority 3: Need to brake for station/rail end
	double brakingDist = ctx->getBrakingDistance(train);
	double distRemaining = ctx->getDistanceToRailEnd(train);
	
	if (distRemaining <= brakingDist * SafetyConstants::BRAKING_MARGIN)
	{
		return ctx->states().braking();
	}
	
	return nullptr;
}

std::string CruisingState::getName() const
{
	return "Cruising";
}
#include "patterns/states/AcceleratingState.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "simulation/SafetyConstants.hpp"
#include "simulation/RiskData.hpp"

void AcceleratingState::update(Train* train, double dt)
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
	
	double accelForceN = PhysicsSystem::kNtoN(train->getMaxAccelForce());
	double netForce = PhysicsSystem::calculateNetForce(train, accelForceN);
	
	PhysicsSystem::updateVelocity(train, netForce, dt);
	
	double speedLimitMs = PhysicsSystem::kmhToMs(currentRail->getSpeedLimit());
	if (train->getVelocity() > speedLimitMs)
	{
		train->setVelocity(speedLimitMs);
	}
	
	PhysicsSystem::updatePosition(train, dt);
}

ITrainState* AcceleratingState::checkTransition(Train* train, SimulationContext* ctx)
{
	if (!train || !ctx)
	{
		return nullptr;
	}
	
	const RiskData& risk = ctx->getRisk(train);
	
	// Priority 1: Emergency zone (collision imminent)
	if (SafetyConstants::isEmergencyZone(risk.gap, risk.brakingDistance))
	{
		return ctx->states().emergency();
	}
	
	// Priority 2: Following zone (need to maintain spacing)
	if (SafetyConstants::isFollowingZone(risk.gap, risk.brakingDistance, risk.safeDistance))
	{
		return ctx->states().waiting();
	}
	
	// Priority 3: Reached speed limit → Cruising
	double speedLimitMs = ctx->getCurrentRailSpeedLimit(train);
	if (train->getVelocity() >= speedLimitMs * 0.99)
	{
		return ctx->states().cruising();
	}
	
	return nullptr;
}

std::string AcceleratingState::getName() const
{
	return "Accelerating";
}
#include "patterns/states/EmergencyState.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "core/Train.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "simulation/SafetyConstants.hpp"
#include "simulation/RiskData.hpp"

void EmergencyState::update(Train* train, double dt)
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
	
	if (train->getVelocity() < 0.0)
	{
		train->setVelocity(0.0);
	}
}

ITrainState* EmergencyState::checkTransition(Train* train, SimulationContext* ctx)
{
	if (!train || !ctx)
	{
		return nullptr;
	}
	
	const RiskData& risk = ctx->getRisk(train);
	
	if (train->getVelocity() <= 0.1)
	{
		return ctx->states().waiting();
	}
	
	if (!risk.hasLeader())
	{
		return ctx->states().accelerating();
	}
	
	double exitThreshold = risk.safeDistance * SafetyConstants::EMERGENCY_EXIT_HYSTERESIS;
	
	if (risk.gap > exitThreshold)
	{
		return ctx->states().waiting();
	}
	
	double ttc = risk.timeToCollision();
	if (ttc > SafetyConstants::EMERGENCY_TTC_THRESHOLD && risk.gap > risk.safeDistance)
	{
		return ctx->states().waiting();
	}
	
	return nullptr;
}

std::string EmergencyState::getName() const
{
	return "Emergency";
}
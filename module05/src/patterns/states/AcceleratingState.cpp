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

    // 1. Emergency situation always has the highest priority
    if (SafetyConstants::isEmergencyZone(risk.gap, risk.brakingDistance))
    {
        return ctx->states().emergency();
    }

    // 2. If there is a leader train ahead
    if (risk.hasLeader())
    {
        // If the leader has stopped, the follower must start braking
        if (risk.leader->getVelocity() < 0.1)
        {
            return ctx->states().braking();
        }

        // If the gap to the moving leader is too small, also start braking
        if (risk.gap < risk.safeDistance)
        {
            return ctx->states().braking();
        }
    }

    // 3. If the train has reached (or is very close to) the speed limit, switch to cruising
    double speedLimitMs = ctx->getCurrentRailSpeedLimit(train);

    if (train->getVelocity() >= speedLimitMs * 0.99)
    {
        return ctx->states().cruising();
    }

    // Otherwise remain in the accelerating state
    return nullptr;
}

std::string AcceleratingState::getName() const
{
	return "Accelerating";
}
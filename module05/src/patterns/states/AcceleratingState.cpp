#include "patterns/states/AcceleratingState.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "patterns/mediator/TrafficController.hpp"
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

    // 1. Emergency situation always has the highest priority (safety override)
    if (SafetyConstants::isEmergencyZone(risk.gap, risk.brakingDistance))
    {
        return ctx->states().emergency();
    }

    // 2. If there is a leader train ahead
    if (risk.hasLeader())
    {
        // Check priority through TrafficController
        TrafficController* controller = ctx->getTrafficController();
        Rail* currentRail = train->getCurrentRail();
        
        if (controller && currentRail)
        {
            // Request access - if GRANTED, we have higher priority than leader
            TrafficController::AccessDecision decision = 
                controller->requestRailAccess(train, currentRail);
            
            if (decision == TrafficController::DENY)
            {
                // Leader has higher priority - must yield
                // If the leader has stopped, start braking
                if (risk.leader->getVelocity() < 0.1)
                {
                    return ctx->states().braking();
                }

                // If gap to moving leader is too small, also start braking
                if (risk.gap < risk.safeDistance)
                {
                    return ctx->states().braking();
                }
            }
            // If GRANT: we have priority, ignore leader and continue
        }
        else
        {
            // Fallback: no controller, use old logic
            if (risk.leader->getVelocity() < 0.1)
            {
                return ctx->states().braking();
            }

            if (risk.gap < risk.safeDistance)
            {
                return ctx->states().braking();
            }
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
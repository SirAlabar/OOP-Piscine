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
        // Exceeding limit - apply light braking
        double frictionForce = PhysicsSystem::calculateFriction(train);
        double netForce = -frictionForce;

        PhysicsSystem::updateVelocity(train, netForce, dt);
    }
    else if (currentVelocity < speedLimitMs * 0.95)
    {
        // Below limit - accelerate
        double accelForceN = PhysicsSystem::kNtoN(train->getMaxAccelForce());
        double netForce = PhysicsSystem::calculateNetForce(train, accelForceN);

        PhysicsSystem::updateVelocity(train, netForce, dt);

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

    // 1. Emergency situations always have highest priority
    if (SafetyConstants::isEmergencyZone(risk.gap, risk.brakingDistance))
    {
        return ctx->states().emergency();
    }

    // 2. Check if there is a leader ahead
    if (risk.hasLeader())
    {
        // If the leader has stopped, transition to Braking first
        if (risk.leader->getVelocity() < 0.1)
        {
            return ctx->states().braking();
        }

        // If too close to a moving leader, also start braking
        if (risk.gap < risk.safeDistance)
        {
            return ctx->states().braking();
        }
    }

    // 3. Check if braking is required due to approaching end of the current rail
    double brakingDist = ctx->getBrakingDistance(train);
    double distRemaining = ctx->getDistanceToRailEnd(train);

    if (distRemaining <= brakingDist * SafetyConstants::BRAKING_MARGIN)
    {
        return ctx->states().braking();
    }

    // Otherwise, remain in Cruising state
    return nullptr;
}

std::string CruisingState::getName() const
{
	return "Cruising";
}
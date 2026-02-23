#include "patterns/behavioral/states/CruisingState.hpp"
#include "patterns/behavioral/states/StateRegistry.hpp"
#include "patterns/behavioral/states/TrainTransitionState.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "simulation/core/SimulationContext.hpp"
#include "simulation/systems/PhysicsSystem.hpp"
#include "simulation/physics/SafetyConstants.hpp"
#include "simulation/physics/RiskData.hpp"

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

    double speedLimitMs    = PhysicsSystem::kmhToMs(currentRail->getSpeedLimit());
    double currentVelocity = train->getVelocity();

    if (currentVelocity > speedLimitMs)
    {
        // Exceeding limit — apply light braking.
        double netForce = -PhysicsSystem::calculateFriction(train);
        PhysicsSystem::updateVelocity(train, netForce, dt);
    }
    else if (currentVelocity < speedLimitMs * 0.95)
    {
        // Below limit — re-accelerate.
        double accelForceN = PhysicsSystem::kNtoN(train->getMaxAccelForce());
        double netForce    = PhysicsSystem::calculateNetForce(train, accelForceN);
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

    // 1. Emergency zone — highest priority safety override.
    if (SafetyConstants::isEmergencyZone(risk.gap, risk.brakingDistance))
    {
        return ctx->states().emergency();
    }

    // 2. Leader interaction — yield if priority demands it.
    ITrainState* leaderResult = TrainTransitionState::checkLeaderInteraction(train, ctx);
    if (leaderResult)
    {
        return leaderResult;
    }

    // 3. Approaching end of rail — begin braking.
    double brakingDist    = ctx->getBrakingDistance(train);
    double distRemaining  = ctx->getDistanceToRailEnd(train);

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
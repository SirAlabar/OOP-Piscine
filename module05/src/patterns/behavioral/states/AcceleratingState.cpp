#include "patterns/behavioral/states/AcceleratingState.hpp"
#include "patterns/behavioral/states/StateRegistry.hpp"
#include "patterns/behavioral/states/TrainTransitionState.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "simulation/core/SimulationContext.hpp"
#include "simulation/systems/PhysicsSystem.hpp"
#include "simulation/physics/SafetyConstants.hpp"
#include "simulation/physics/RiskData.hpp"

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
    double netForce    = PhysicsSystem::calculateNetForce(train, accelForceN);

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

    // 3. Speed limit reached — switch to cruising.
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
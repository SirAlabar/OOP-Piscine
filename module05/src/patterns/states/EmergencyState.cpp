#include "patterns/states/EmergencyState.hpp"
#include "patterns/states/StoppedState.hpp"
#include "patterns/states/WaitingState.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/RiskData.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "core/Train.hpp"

ITrainState* EmergencyState::checkTransition(Train* train, const SimulationContext* ctx)
{
    if (!train || !ctx) return nullptr;

    const RiskData& risk = ctx->getRisk(train);

    if (train->getVelocity() <= 0.1)
        return ctx->states().stopped();

    double ttc = risk.timeToCollision();

    // Leave emergency once collision is no longer imminent
    if (!risk.hasLeader() || ttc < 0.0 || (ttc > 3.0 && risk.gap > risk.safeDistance))
        return ctx->states().waiting();

    return nullptr;
}



void EmergencyState::update(Train* train, double dt)
{
    if (!train) return;

    // SimulationContext* ctx = train->getContext();
    if (!ctx) return;

    double brake = -PhysicsSystem::kNtoN(train->getMaxBrakeForce());
    ctx->applyForce(train, brake, dt);
}


std::string EmergencyState::getName() const
{
	return "Emergency";
}
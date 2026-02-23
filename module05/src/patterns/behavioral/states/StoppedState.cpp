#include "patterns/behavioral/states/StoppedState.hpp"
#include "patterns/behavioral/states/StateRegistry.hpp"
#include "patterns/behavioral/states/TrainTransitionState.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "simulation/core/SimulationContext.hpp"

void StoppedState::update(Train* train, double dt)
{
    (void)dt;

    if (!train)
    {
        return;
    }

    train->setVelocity(0.0);
}

ITrainState* StoppedState::checkTransition(Train* train, SimulationContext* ctx)
{
    if (!train || !ctx)
    {
        return nullptr;
    }

    train->setVelocity(0.0);

    // Must wait until the station stop duration expires (decremented by SimulationManager).
    if (ctx->getStopDuration(train) > 0.0)
    {
        return nullptr;
    }

    // Request access — on GRANT resume, on DENY transition to Waiting.
    ITrainState* resumed = TrainTransitionState::checkRailAccessForResume(train, ctx);
    if (resumed)
    {
        return resumed;
    }

    return ctx->states().waiting();
}

std::string StoppedState::getName() const
{
    return "Stopped";
}
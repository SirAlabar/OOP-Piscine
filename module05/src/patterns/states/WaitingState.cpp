#include "patterns/states/WaitingState.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "patterns/states/TrainTransitionState.hpp"
#include "core/Train.hpp"
#include "simulation/SimulationContext.hpp"

void WaitingState::update(Train* train, double dt)
{
    (void)dt;

    if (!train)
    {
        return;
    }

    train->setVelocity(0.0);
}

ITrainState* WaitingState::checkTransition(Train* train, SimulationContext* ctx)
{
    if (!train || !ctx)
    {
        return nullptr;
    }

    // Resume if the TrafficController now grants access; otherwise keep waiting.
    return TrainTransitionState::checkRailAccessForResume(train, ctx);
}

std::string WaitingState::getName() const
{
    return "Waiting";
}
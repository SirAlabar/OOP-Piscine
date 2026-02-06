#include "patterns/states/WaitingState.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "patterns/mediator/TrafficController.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "simulation/SafetyConstants.hpp"
#include "simulation/RiskData.hpp"
#include <algorithm>

void WaitingState::update(Train* train, double dt)
{
    (void)dt;

    if (!train)
    {
        return;
    }

    // Waiting is a logical state only: the train must remain stopped
    train->setVelocity(0.0);
}

ITrainState* WaitingState::checkTransition(Train* train, SimulationContext* ctx)
{
    if (!train || !ctx)
    {
        return nullptr;
    }

    // Get current rail
    Rail* currentRail = train->getCurrentRail();
    if (!currentRail)
    {
        return nullptr;
    }

    // Request access through TrafficController (Mediator pattern)
    TrafficController* controller = ctx->getTrafficController();
    if (!controller)
    {
        return nullptr;
    }

    // Check if we have permission to resume movement
    if (controller->requestRailAccess(train, currentRail) == TrafficController::GRANT)
    {
        return ctx->states().accelerating();
    }

    // Access denied - remain in Waiting state
    return nullptr;
}

std::string WaitingState::getName() const
{
	return "Waiting";
}
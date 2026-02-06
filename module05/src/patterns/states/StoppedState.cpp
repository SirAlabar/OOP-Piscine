#include "patterns/states/StoppedState.hpp"
#include "patterns/mediator/TrafficController.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "simulation/SimulationContext.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "simulation/RiskData.hpp"

void StoppedState::update(Train* train, double dt)
{
    (void)dt;

	if (!train)
	{
		return;
	}
	
	// Ensure velocity is zero while stopped
	train->setVelocity(0.0);
}

ITrainState* StoppedState::checkTransition(Train* train, SimulationContext* ctx)
{
    if (!train || !ctx)
    {
        return nullptr;
    }

    // Ensure the train is fully stopped
    train->setVelocity(0.0);

    // Check if train has a stop duration (station stop)
    double stopDuration = ctx->getStopDuration(train);
    if (stopDuration > 0.0)
    {
        // Train must wait until stop duration expires
        // Decrement will happen in SimulationManager
        return nullptr;  // Stay in Stopped state
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
    TrafficController::AccessDecision decision = controller->requestRailAccess(train, currentRail);
    
    if (decision == TrafficController::GRANT)
    {
        return ctx->states().accelerating();
    }
    
    // Access denied - transition to Waiting state
    return ctx->states().waiting();
}

std::string StoppedState::getName() const
{
	return "Stopped";
}
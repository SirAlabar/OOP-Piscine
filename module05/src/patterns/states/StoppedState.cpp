#include "patterns/states/StoppedState.hpp"
#include "core/Train.hpp"
#include "simulation/SimulationContext.hpp"
#include "patterns/states/StateRegistry.hpp"

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
	
	// Decrement stop duration
	bool expired = ctx->decrementStopDuration(train, 1.0);
	
	if (expired)
	{
		// Clear duration and transition to Accelerating
		ctx->clearStopDuration(train);
		return ctx->states().accelerating();
	}
	
	return nullptr;
}

std::string StoppedState::getName() const
{
	return "Stopped";
}
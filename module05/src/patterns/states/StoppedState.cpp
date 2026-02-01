#include "patterns/states/StoppedState.hpp"
#include "patterns/states/AcceleratingState.hpp"
#include "core/Train.hpp"
#include "simulation/SimulationContext.hpp"

StoppedState::StoppedState(double stopDuration)
	: _timeRemaining(stopDuration)
{
}

void StoppedState::update(Train* train, double dt)
{
	if (!train)
	{
		return;
	}
	
	// Stopped: velocity = 0, countdown timer
	train->setVelocity(0.0);
	
	_timeRemaining -= dt;
	if (_timeRemaining < 0.0)
	{
		_timeRemaining = 0.0;
	}
}

ITrainState* StoppedState::checkTransition(Train* train, SimulationContext* ctx)
{
	if (!train || !ctx)
	{
		return nullptr;
	}
	
	// Transition: Stopped → Accelerating (stop duration elapsed)
	if (_timeRemaining <= 0.0)
	{
		ctx->releaseStoppedState(train); // Clean up this state
		
		static AcceleratingState accelState;
		return &accelState;
	}
	
	return nullptr;
}

std::string StoppedState::getName() const
{
	return "Stopped";
}

double StoppedState::getTimeRemaining() const
{
	return _timeRemaining;
}
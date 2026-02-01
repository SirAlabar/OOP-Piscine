#include "patterns/states/StoppedState.hpp"
#include "core/Train.hpp"

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

std::string StoppedState::getName() const
{
	return "Stopped";
}

double StoppedState::getTimeRemaining() const
{
	return _timeRemaining;
}
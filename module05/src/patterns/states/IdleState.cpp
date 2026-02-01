#include "patterns/states/IdleState.hpp"
#include "core/Train.hpp"

void IdleState::update(Train* train, double dt)
{
	(void)dt;

	if (!train)
	{
		return;
	}
	
	// Idle: do nothing, velocity stays at 0
	train->setVelocity(0.0);
}

std::string IdleState::getName() const
{
	return "Idle";
}
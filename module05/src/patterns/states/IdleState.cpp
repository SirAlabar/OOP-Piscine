#include "patterns/states/IdleState.hpp"
#include "core/Train.hpp"
#include "simulation/SimulationContext.hpp"

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


ITrainState* IdleState::checkTransition(Train* train, SimulationContext* ctx)
{
	(void)train;
	(void)ctx;
	
	// IdleState never self-transitions
	// Transition handled by SimulationManager::checkDepartures()
	return nullptr;
}

std::string IdleState::getName() const
{
	return "Idle";
}
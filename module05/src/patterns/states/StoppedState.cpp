#include "patterns/states/StoppedState.hpp"
#include "core/Train.hpp"
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

    const RiskData& risk = ctx->getRisk(train);

    // Ensure the train is fully stopped
    train->setVelocity(0.0);

    // If there is no leader ahead, movement can resume
    if (!risk.hasLeader())
    {
        return ctx->states().accelerating();
    }

    // If the leader is still stopped, transition to Waiting state
    if (risk.leader->getVelocity() < 0.1)
    {
        return ctx->states().waiting();
    }

    // Leader started moving again, resume movement
    return ctx->states().accelerating();
}

std::string StoppedState::getName() const
{
	return "Stopped";
}
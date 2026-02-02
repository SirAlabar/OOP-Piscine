#include "patterns/states/WaitingState.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "core/Train.hpp"
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

    const RiskData& risk = ctx->getRisk(train);

    // If there is no leader ahead anymore, the train can resume movement
    if (!risk.hasLeader())
    {
        return ctx->states().accelerating();
    }

    // If the leader has started moving again, resume movement
    if (risk.leader->getVelocity() > 0.1)
    {
        return ctx->states().accelerating();
    }

    // While the leader remains stopped, stay in Waiting state
    return nullptr;
}

std::string WaitingState::getName() const
{
	return "Waiting";
}
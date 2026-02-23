#include "patterns/behavioral/states/BrakingState.hpp"
#include "patterns/behavioral/states/StateRegistry.hpp"
#include "core/Train.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "simulation/core/SimulationContext.hpp"
#include "simulation/systems/PhysicsSystem.hpp"
#include "simulation/physics/RiskData.hpp"

void BrakingState::update(Train* train, double dt)
{
	if (!train)
	{
		return;
	}
	
	double brakeForceN = PhysicsSystem::kNtoN(train->getMaxBrakeForce());
	double frictionForce = PhysicsSystem::calculateFriction(train);
	
	double netForce = -(brakeForceN + frictionForce);
	
	PhysicsSystem::updateVelocity(train, netForce, dt);
	PhysicsSystem::updatePosition(train, dt);
	
	if (train->getVelocity() <= 0.01)
	{
		train->setVelocity(0.0);
	}
}

ITrainState* BrakingState::checkTransition(Train* train, SimulationContext* ctx)
{
    if (!train || !ctx)
    {
        return nullptr;
    }

    const RiskData& risk = ctx->getRisk(train);

    // 1. If the train is still moving, remain in Braking state
    if (train->getVelocity() > 0.1)
    {
        // While braking, if the situation becomes critical, switch to Emergency
        if (risk.hasLeader() && risk.gap < risk.safeDistance)
        {
            return ctx->states().emergency();
        }

        return nullptr;
    }

    // From this point on, velocity is considered to be effectively zero

    train->setVelocity(0.0);

    // 2. If there is a leader and the distance is not safe, go to Emergency
    if (risk.hasLeader() && risk.gap < risk.safeDistance)
    {
        return ctx->states().emergency();
    }

    // 3. Otherwise, the train has stopped safely
    return ctx->states().stopped();
}

std::string BrakingState::getName() const
{
	return "Braking";
}
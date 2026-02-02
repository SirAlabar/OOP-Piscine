#include "patterns/states/EmergencyState.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "core/Train.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "simulation/SafetyConstants.hpp"
#include "simulation/RiskData.hpp"

void EmergencyState::update(Train* train, double dt)
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
	
	if (train->getVelocity() < 0.0)
	{
		train->setVelocity(0.0);
	}
}

ITrainState* EmergencyState::checkTransition(Train* train, SimulationContext* ctx)
{
    if (!train || !ctx)
    {
        return nullptr;
    }

    // While the train still has velocity, remain in Emergency state
    if (train->getVelocity() > 0.1)
    {
        return nullptr;
    }

    // Velocity is now effectively zero
    train->setVelocity(0.0);

    // After stopping, transition to Stopped state regardless of gap
    return ctx->states().stopped();
}

std::string EmergencyState::getName() const
{
	return "Emergency";
}
#include "simulation/SimulationContext.hpp"
#include "simulation/CollisionAvoidance.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "simulation/RiskData.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "core/Graph.hpp"

SimulationContext::SimulationContext(
	Graph* network,
	CollisionAvoidance* collisionSystem,
	const std::vector<Train*>* trains)
	: _network(network),
	  _collisionSystem(collisionSystem),
	  _trains(trains),
	  _states(new StateRegistry())
{
}

SimulationContext::~SimulationContext()
{
	delete _states;
}

const RiskData& SimulationContext::getRisk(const Train* train) const
{
	auto it = _riskMap.find(const_cast<Train*>(train));
	if (it == _riskMap.end())
	{
		// Return default risk data if not found (shouldn't happen)
		static RiskData defaultRisk;
		return defaultRisk;
	}
	return it->second;
}

void SimulationContext::refreshAllRiskData()
{
	if (!_collisionSystem || !_trains)
	{
		return;
	}

	_riskMap.clear();

	for (Train* train : *_trains)
	{
		if (train && train->getCurrentRail())
		{
			RiskData risk = _collisionSystem->assessRisk(train, *_trains);
			_riskMap[train] = risk;
		}
	}
}

// ===== PHYSICS QUERIES =====

double SimulationContext::getCurrentRailSpeedLimit(const Train* train) const
{
	if (!train || !train->getCurrentRail())
	{
		return 0.0;
	}

	return PhysicsSystem::kmhToMs(train->getCurrentRail()->getSpeedLimit());
}

double SimulationContext::getCurrentRailLength(const Train* train) const
{
	if (!train || !train->getCurrentRail())
	{
		return 0.0;
	}

	return PhysicsSystem::kmToM(train->getCurrentRail()->getLength());
}

double SimulationContext::getBrakingDistance(const Train* train) const
{
	return train ? PhysicsSystem::calculateBrakingDistance(train) : 0.0;
}

double SimulationContext::getDistanceToRailEnd(const Train* train) const
{
	return getCurrentRailLength(train) - train->getPosition();
}

Node* SimulationContext::getCurrentArrivalNode(const Train* train) const
{
	if (!train || !train->getCurrentRail())
	{
		return nullptr;
	}

	return train->getCurrentRail()->getNodeB();
}

StateRegistry& SimulationContext::states()
{
	return *_states;
}

void SimulationContext::setStopDuration(Train* train, double durationSeconds)
{
	if (train)
	{
		_stopDurations[train] = durationSeconds;
	}
}

double SimulationContext::getStopDuration(const Train* train) const
{
	auto it = _stopDurations.find(const_cast<Train*>(train));
	if (it != _stopDurations.end())
	{
		return it->second;
	}
	return 0.0;
}

bool SimulationContext::decrementStopDuration(Train* train, double dt)
{
	auto it = _stopDurations.find(train);
	if (it != _stopDurations.end())
	{
		it->second -= dt;
		if (it->second <= 0.0)
		{
			it->second = 0.0;
			return true;  // Duration expired
		}
	}
	return false;
}

void SimulationContext::clearStopDuration(Train* train)
{
	_stopDurations.erase(train);
}

void SimulationContext::applyForce(Train* train, double force, double dt)
{
	if (!train)
	{
		return;
	}

	double netForce = PhysicsSystem::calculateNetForce(train, force);
	PhysicsSystem::updateVelocity(train, netForce, dt);
	PhysicsSystem::updatePosition(train, dt);
}
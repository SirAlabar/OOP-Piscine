#include "simulation/SimulationContext.hpp"
#include "simulation/CollisionAvoidance.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "patterns/states/StoppedState.hpp"

SimulationContext::SimulationContext(
	Graph* network,
	CollisionAvoidance* collisionSystem,
	const std::vector<Train*>* trains)
	: _network(network),
	  _collisionSystem(collisionSystem),
	  _trains(trains)
{
}

SimulationContext::~SimulationContext()
{
	for (auto& pair : _stoppedStates)
	{
		delete pair.second;
	}
	_stoppedStates.clear();
}

// ─────────── PHYSICS QUERIES ───────────

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

double SimulationContext::distanceToNextTrain(const Train* train) const
{
	return (_collisionSystem && _trains)
		? _collisionSystem->distanceToNextTrain(train, *_trains)
		: -1.0;
}

double SimulationContext::getMinimumSafeDistance(const Train* train) const
{
	return _collisionSystem
		? _collisionSystem->getMinimumSafeDistance(train)
		: 0.0;
}

StoppedState* SimulationContext::getOrCreateStoppedState(Train* train, double durationSeconds)
{
	if (!train)
    {
		return nullptr;
    }

	auto it = _stoppedStates.find(train);
	if (it != _stoppedStates.end())
	{
		delete it->second;
	}

	_stoppedStates[train] = new StoppedState(durationSeconds);
	return _stoppedStates[train];
}

void SimulationContext::releaseStoppedState(Train* train)
{
	auto it = _stoppedStates.find(train);
	if (it != _stoppedStates.end())
	{
		delete it->second;
		_stoppedStates.erase(it);
	}
}

const RiskData& SimulationContext::getRisk(const Train* train) const
{
    return _riskMap.at(train);
}

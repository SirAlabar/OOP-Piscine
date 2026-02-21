#include "simulation/SimulationContext.hpp"
#include "simulation/CollisionAvoidance.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "simulation/RiskData.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "patterns/mediator/ITrainController.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "core/Graph.hpp"
#include <iostream>

SimulationContext::SimulationContext(
    Graph*                     network,
    CollisionAvoidance*        collisionSystem,
    const std::vector<Train*>* trains,
    ITrainController*          trafficController)
    : _network(network),
      _collisionSystem(collisionSystem),
      _trafficController(trafficController),
      _trains(trains),
      _states()
{
}

SimulationContext::~SimulationContext() = default;

// Risk data

const RiskData& SimulationContext::getRisk(const Train* train) const
{
    auto it = _riskMap.find(const_cast<Train*>(train));

    if (it == _riskMap.end())
    {
        static const RiskData sentinel;
        std::cerr << "[SimulationContext] WARNING: getRisk() miss — "
                     "returning sentinel RiskData\n";
        return sentinel;
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
            _riskMap[train] = _collisionSystem->assessRisk(train, *_trains);
        }
    }
}

// IPhysicsQueries

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
    if (!train)
    {
        return 0.0;
    }

    return getCurrentRailLength(train) - train->getPosition();
}

Node* SimulationContext::getCurrentArrivalNode(const Train* train) const
{
    if (!train || !train->getCurrentRail())
    {
        return nullptr;
    }

    const auto& path         = train->getPath();
    std::size_t currentIndex = train->getCurrentRailIndex();

    if (currentIndex >= path.size())
    {
        return nullptr;
    }

    return path[currentIndex].to;
}

// IStopTimerStore

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
            return true;
        }
    }
    return false;
}

void SimulationContext::clearStopDuration(Train* train)
{
    _stopDurations.erase(train);
}

// State catalog
StateRegistry& SimulationContext::states()
{
    return _states;
}

const StateRegistry& SimulationContext::states() const
{
    return _states;
}

// Traffic access
ITrainController* SimulationContext::getTrafficController() const
{
    return _trafficController;
}

// Physics mutation
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
#include "simulation/core/SimulationContext.hpp"
#include "simulation/interfaces/ICollisionAvoidance.hpp"
#include "simulation/systems/PhysicsSystem.hpp"
#include "simulation/physics/RiskData.hpp"
#include "patterns/behavioral/states/StateRegistry.hpp"
#include "patterns/behavioral/mediator/ITrainController.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "core/Graph.hpp"
#include <iostream>

SimulationContext::SimulationContext(Graph*                     network,
                                     ICollisionAvoidance*        collisionSystem,
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

bool SimulationContext::isTrainActive(const Train* train) const
{
    return train
        && train->getCurrentState()
        && !train->isFinished()
        && train->getCurrentState() != const_cast<StateRegistry&>(_states).idle();
}

bool SimulationContext::hasAnyActiveTrain() const
{
    if (!_trains)
    {
        return false;
    }

    for (const Train* train : *_trains)
    {
        if (isTrainActive(train))
        {
            return true;
        }
    }

    return false;
}

const RiskData& SimulationContext::getRisk(const Train* train) const
{
    auto it = _riskMap.find(const_cast<Train*>(train));

    if (it == _riskMap.end())
    {
        static const RiskData sentinel;
        std::cerr << "[SimulationContext] getRisk() miss — returning sentinel\n";
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
    return (it != _stopDurations.end()) ? it->second : 0.0;
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

StateRegistry& SimulationContext::states()
{
    return _states;
}

const StateRegistry& SimulationContext::states() const
{
    return _states;
}

ITrainController* SimulationContext::getTrafficController() const
{
    return _trafficController;
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
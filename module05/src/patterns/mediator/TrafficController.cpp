#include "patterns/mediator/TrafficController.hpp"
#include "simulation/ICollisionAvoidance.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "simulation/SafetyConstants.hpp"
#include <cmath>

TrafficController::TrafficController(const Graph*               network,
                                     ICollisionAvoidance*       collisionSystem,
                                     const std::vector<Train*>* allTrains)
    : _network(network),
      _collisionSystem(collisionSystem),
      _allTrains(allTrains)
{
}

TrafficController::AccessDecision TrafficController::requestRailAccess(Train* train, Rail* targetRail)
{
    if (!train || !targetRail)
    {
        return DENY;
    }

    if (train->isFinished())
    {
        return DENY;
    }

    Train* conflictingTrain = findConflictingTrain(train, targetRail);

    if (!conflictingTrain)
    {
        return GRANT;
    }

    if (hasSufficientGap(train, targetRail))
    {
        return GRANT;
    }

    if (hasHigherPriority(train, conflictingTrain))
    {
        return GRANT;
    }

    return DENY;
}

bool TrafficController::hasSufficientGap(Train* train, Rail* rail) const
{
    if (!train || !rail)
    {
        return false;
    }

    Train* leader = findConflictingTrain(train, rail);

    if (!leader)
    {
        return true;
    }

    double gap          = calculateGapToLeader(train, leader, rail);
    double safeDistance = SafetyConstants::MINIMUM_CLEARANCE
                        + (train->getVelocity() * SafetyConstants::SAFE_TIME_HEADWAY);

    return gap >= safeDistance;
}

bool TrafficController::hasHigherPriority(Train* requestingTrain, Train* conflictingTrain) const
{
    if (!requestingTrain || !conflictingTrain)
    {
        return false;
    }

    if (requestingTrain->getDepartureTime() < conflictingTrain->getDepartureTime())
    {
        return true;
    }

    if (requestingTrain->getDepartureTime() == conflictingTrain->getDepartureTime())
    {
        return requestingTrain->getID() < conflictingTrain->getID();
    }

    return false;
}

Train* TrafficController::findConflictingTrain(Train* train, Rail* rail) const
{
    if (!train || !rail || !_allTrains)
    {
        return nullptr;
    }

    Train* nearestLeader = nullptr;
    double minGap        = 999999.0;

    for (Train* other : *_allTrains)
    {
        if (!other || other == train || other->isFinished())
        {
            continue;
        }

        if (other->getCurrentRail() != rail)
        {
            continue;
        }

        if (!areTravelingSameDirection(train, other, rail))
        {
            continue;
        }

        double gap = calculateGapToLeader(train, other, rail);

        if (gap > 0.0 && gap < minGap)
        {
            minGap        = gap;
            nearestLeader = other;
        }
    }

    return nearestLeader;
}

double TrafficController::calculateGapToLeader(Train* train, Train* leader, Rail* rail) const
{
    if (!train || !leader || !rail)
    {
        return 999999.0;
    }

    if (train->getCurrentRail() != rail)
    {
        return leader->getPosition();
    }

    return leader->getPosition() - train->getPosition();
}

bool TrafficController::areTravelingSameDirection(Train* train, Train* other, Rail* rail) const
{
    if (!train || !other || !rail)
    {
        return false;
    }

    const auto& trainPath = train->getPath();
    const auto& otherPath = other->getPath();

    std::size_t trainRailIdx = static_cast<std::size_t>(-1);

    for (std::size_t i = 0; i < trainPath.size(); ++i)
    {
        if (trainPath[i].rail == rail)
        {
            trainRailIdx = i;
            break;
        }
    }

    std::size_t otherRailIdx = static_cast<std::size_t>(-1);

    for (std::size_t i = 0; i < otherPath.size(); ++i)
    {
        if (otherPath[i].rail == rail)
        {
            otherRailIdx = i;
            break;
        }
    }

    if (trainRailIdx == static_cast<std::size_t>(-1) || otherRailIdx == static_cast<std::size_t>(-1))
    {
        return false;
    }

    return (trainPath[trainRailIdx].from == otherPath[otherRailIdx].from
         && trainPath[trainRailIdx].to   == otherPath[otherRailIdx].to);
}
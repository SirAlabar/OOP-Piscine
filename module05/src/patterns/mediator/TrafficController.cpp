#include "patterns/mediator/TrafficController.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "simulation/CollisionAvoidance.hpp"
#include "simulation/SafetyConstants.hpp"
#include <cmath>

TrafficController::TrafficController(const Graph* network,
                                     CollisionAvoidance* collisionSystem,
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

	// If train is finished, deny access
	if (train->isFinished())
	{
		return DENY;
	}

	// Check if there are conflicting trains on this rail
	Train* conflictingTrain = findConflictingTrain(train, targetRail);

	// No conflict → grant access
	if (!conflictingTrain)
	{
		return GRANT;
	}

	// Conflict exists → check gap
	if (hasSufficientGap(train, targetRail))
	{
		return GRANT;  // Safe distance maintained
	}

	// Insufficient gap → apply priority rules
	if (hasHigherPriority(train, conflictingTrain))
	{
		return GRANT;  // Requesting train has priority
	}

	return DENY;  // Conflicting train has priority
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
		return true;  // No leader → sufficient gap
	}

	double gap = calculateGapToLeader(train, leader, rail);
	double safeDistance = SafetyConstants::MINIMUM_CLEARANCE + 
	                      (train->getVelocity() * SafetyConstants::SAFE_TIME_HEADWAY);

	return gap >= safeDistance;
}

bool TrafficController::hasHigherPriority(Train* requestingTrain, Train* conflictingTrain) const
{
	if (!requestingTrain || !conflictingTrain)
	{
		return false;
	}

	// Priority rule: earlier departure time wins
	if (requestingTrain->getDepartureTime() < conflictingTrain->getDepartureTime())
	{
		return true;
	}

	// If same departure time, use train ID as tie-breaker
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
	double minGap = 999999.0;

	for (Train* other : *_allTrains)
	{
		if (!other || other == train || other->isFinished())
		{
			continue;
		}

		// Check if other train is on the target rail
		if (other->getCurrentRail() != rail)
		{
			continue;
		}

		// Check if traveling in same direction
		if (!areTravelingSameDirection(train, other, rail))
		{
			continue;  // Opposite direction → no conflict
		}

		// Calculate gap
		double gap = calculateGapToLeader(train, other, rail);

		// Only consider trains ahead (positive gap)
		if (gap > 0.0 && gap < minGap)
		{
			minGap = gap;
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

	// If train not yet on rail, gap is from rail start to leader position
	if (train->getCurrentRail() != rail)
	{
		return leader->getPosition();
	}

	// Both on same rail: simple subtraction
	return leader->getPosition() - train->getPosition();
}

bool TrafficController::areTravelingSameDirection(Train* train, Train* other, Rail* rail) const
{
	if (!train || !other || !rail)
	{
		return false;
	}

	// Get train's path
	const auto& trainPath = train->getPath();
	const auto& otherPath = other->getPath();

	// Find rail in train's path
	size_t trainRailIdx = static_cast<size_t>(-1);
	for (size_t i = 0; i < trainPath.size(); ++i)
	{
		if (trainPath[i].rail == rail)
		{
			trainRailIdx = i;
			break;
		}
	}

	// Find rail in other's path
	size_t otherRailIdx = static_cast<size_t>(-1);
	for (size_t i = 0; i < otherPath.size(); ++i)
	{
		if (otherPath[i].rail == rail)
		{
			otherRailIdx = i;
			break;
		}
	}

	// If rail not found in either path, assume different direction
	if (trainRailIdx == static_cast<size_t>(-1) || otherRailIdx == static_cast<size_t>(-1))
	{
		return false;
	}

	// Compare direction nodes
	Node* trainFrom = trainPath[trainRailIdx].from;
	Node* trainTo = trainPath[trainRailIdx].to;
	Node* otherFrom = otherPath[otherRailIdx].from;
	Node* otherTo = otherPath[otherRailIdx].to;

	// Same direction if traveling from same node to same node
	return (trainFrom == otherFrom && trainTo == otherTo);
}
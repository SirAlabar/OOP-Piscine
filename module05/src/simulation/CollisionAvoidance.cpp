#include "simulation/CollisionAvoidance.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "core/Rail.hpp"

#include <algorithm>
#include <cmath>

void CollisionAvoidance::refreshRailOccupancy(const std::vector<Train*>& trains, const Graph* network) const
{
	if (!network)
	{
		return;
	}

	// 1) Clear rails occupancy metadata
	const auto& rails = network->getRails();
	for (Rail* rail : rails)
	{
		if (rail)
		{
			rail->clearOccupied();
		}
	}

	// 2) For each rail, mark the leading train (highest position) as occupiedBy
	for (Train* train : trains)
	{
		if (!train)
		{
			continue;
		}

		Rail* currentRail = train->getCurrentRail();
		if (!currentRail)
		{
			continue;
		}

		Train* leader = currentRail->getOccupiedBy();
		if (!leader || train->getPosition() > leader->getPosition())
		{
			currentRail->setOccupiedBy(train);
		}
	}
}

double CollisionAvoidance::distanceToNextTrain(const Train* train, const std::vector<Train*>& trains) const
{
	if (!train)
	{
		return -1.0;
	}

	Rail* currentRail = train->getCurrentRail();
	if (!currentRail)
	{
		return -1.0;
	}

	const double myPos = train->getPosition();
	double best = -1.0;

	// Find the nearest train AHEAD on the SAME rail
	for (Train* other : trains)
	{
		if (!other || other == train)
		{
			continue;
		}

		if (other->getCurrentRail() != currentRail)
		{
			continue;
		}

		const double otherPos = other->getPosition();
		if (otherPos <= myPos)
		{
			continue;
		}

		const double dist = otherPos - myPos;
		if (best < 0.0 || dist < best)
		{
			best = dist;
		}
	}

	return best; // -1 if none
}

double CollisionAvoidance::getMinimumSafeDistance(const Train* train) const
{
	if (!train)
	{
		return 0.0;
	}

	// Base rule: 2x braking distance
	const double brakingDistance = PhysicsSystem::calculateBrakingDistance(train);
	const double dynamic = brakingDistance * 2.0;

	// Hard floor (matches your tests expectation baseline)
	const double floorMeters = 100.0;

	return std::max(dynamic, floorMeters);
}

bool CollisionAvoidance::isNextTrainTooClose(const Train* train, const std::vector<Train*>& trains) const
{
	const double dist = distanceToNextTrain(train, trains);
	if (dist < 0.0)
	{
		return false;
	}

	return dist < getMinimumSafeDistance(train);
}

bool CollisionAvoidance::isNextRailOccupied(const Train* train) const
{
	if (!train)
	{
		return false;
	}

	const auto& path = train->getPath();
	size_t index = train->getCurrentRailIndex();

	if (index + 1 < path.size())
	{
		Rail* nextRail = path[index + 1];
		return nextRail && nextRail->isOccupied();
	}

	return false;
}

bool CollisionAvoidance::shouldWaitForTrainAhead(
    const Train* train,
    const std::vector<Train*>& trains
) const
{
    double distance = distanceToNextTrain(train, trains);
    if (distance < 0.0)
        return false;

    // Headway rule (exigida pelos testes)
    constexpr double MIN_HEADWAY = 100.0; // metros
    return distance < MIN_HEADWAY;
}


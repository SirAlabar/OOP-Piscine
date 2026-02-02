#include "simulation/CollisionAvoidance.hpp"
#include "simulation/RiskData.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "core/Graph.hpp"
#include "simulation/PhysicsSystem.hpp"
#include <limits>

void CollisionAvoidance::refreshRailOccupancy(const std::vector<Train*>& trains, const Graph* network)
{
	if (!network)
	{
		return;
	}
	
	// Clear all occupancy
	for (Rail* rail : network->getRails())
	{
		if (rail)
		{
			rail->clearOccupied();
		}
	}
	
	// Mark current rail as occupied
	for (Train* train : trains)
	{
		if (train && train->getCurrentRail())
		{
			train->getCurrentRail()->setOccupiedBy(train);
		}
	}
}

RiskData CollisionAvoidance::assessRisk(const Train* train, const std::vector<Train*>& allTrains) const
{
	RiskData data;
	
	if (!train)
	{
		return data;
	}
	
	// Find leader on route
	data.leader = findLeaderOnRoute(train, allTrains);
	
	// Calculate gap and closing speed
	if (data.leader)
	{
		data.gap = calculateGap(train, data.leader);
		data.closingSpeed = calculateClosingSpeed(train, data.leader);
	}
	
	// Physical constraints
	data.brakingDistance = calculateBrakingDistance(train);
	data.safeDistance = calculateSafeDistance(train);
	
	// Rail constraints
	data.currentSpeedLimit = getCurrentSpeedLimit(train);
	data.nextSpeedLimit = getNextSpeedLimit(train);
	
	return data;
}

// ===== ROUTE-AWARE HELPER METHODS =====

double CollisionAvoidance::calculateAbsoluteRoutePosition(const Train* train) const
{
	if (!train)
	{
		return 0.0;
	}
	
	const auto& path = train->getPath();
	size_t currentIndex = train->getCurrentRailIndex();
	
	// Sum distance of completed rails
	double totalDistance = 0.0;
	for (size_t i = 0; i < currentIndex && i < path.size(); ++i)
	{
		totalDistance += PhysicsSystem::kmToM(path[i]->getLength());
	}
	
	// Add current position on current rail
	totalDistance += train->getPosition();
	
	return totalDistance;
}

bool CollisionAvoidance::findRailIndexInPath(const Train* t, const Rail* rail, size_t startIndex, size_t& outIndex) const
{
    const auto& path = t->getPath();
    for (size_t i = startIndex; i < path.size(); ++i)
    {
        if (path[i] == rail)
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

Train* CollisionAvoidance::findLeaderOnRoute(const Train* train, const std::vector<Train*>& allTrains) const
{
    if (!train || !train->getCurrentRail())
    {
        return nullptr;
    }

    Train* best = nullptr;
    double bestGap = std::numeric_limits<double>::infinity();

    for (Train* other : allTrains)
    {
        if (!other || other == train)
        {
            continue;
        }

        // Ignore trains that are not active yet
        if (!other->getCurrentRail())
        {
            continue;
        }

        // Ignore trains behind or on different routes
        double gap = calculateGap(train, other);

        if (gap < 0.0)
        {
            continue;
        }

        if (gap < bestGap)
        {
            bestGap = gap;
            best = other;
        }
    }

    return best;
}

double CollisionAvoidance::calculateGap(const Train* train, const Train* leader) const
{
    if (!train || !leader)
    {
        return -1.0;
    }

    Rail* leaderRail = leader->getCurrentRail();
    Rail* myRail = train->getCurrentRail();

    if (!leaderRail || !myRail)
    {
        return -1.0;
    }

    const auto& myPath = train->getPath();
    size_t myIndex = train->getCurrentRailIndex();

    size_t leaderIndex = 0;

    if (!findRailIndexInPath(train, leaderRail, myIndex, leaderIndex))
    {
        return -1.0;
    }

    // Case where both trains are on the same rail
    if (leaderIndex == myIndex)
    {
        double gap = leader->getPosition() - train->getPosition();
        return (gap > 0.0) ? gap : -1.0;
    }

    // Distance from the train position to the end of its current rail
    double gap = PhysicsSystem::kmToM(myPath[myIndex]->getLength()) - train->getPosition();

    // Add the full length of all intermediate rails between the two trains
    for (size_t i = myIndex + 1; i < leaderIndex; ++i)
    {
        gap += PhysicsSystem::kmToM(myPath[i]->getLength());
    }

    // Add the position of the leader on its own rail
    gap += leader->getPosition();

    return (gap > 0.0) ? gap : -1.0;
}

double CollisionAvoidance::calculateClosingSpeed(const Train* train, const Train* leader) const
{
	if (!train || !leader)
	{
		return 0.0;
	}
	
	// Positive = approaching, negative = separating
	return train->getVelocity() - leader->getVelocity();
}

double CollisionAvoidance::calculateBrakingDistance(const Train* train) const
{
	if (!train)
	{
		return 0.0;
	}
	
	return PhysicsSystem::calculateBrakingDistance(train);
}

double CollisionAvoidance::calculateSafeDistance(const Train* train) const
{
	if (!train)
	{
		return 100.0;
	}
	
	// Base calculation: 2-second time headway
	double timeHeadway = 2.0;  // seconds
	double speedBasedMargin = train->getVelocity() * timeHeadway;
	
	// Add braking distance buffer
	double brakingMargin = calculateBrakingDistance(train);
	
	// Minimum clearance
	double minClearance = 50.0;  // meters
	
	return minClearance + speedBasedMargin + brakingMargin;
}

double CollisionAvoidance::getCurrentSpeedLimit(const Train* train) const
{
	if (!train)
	{
		return 0.0;
	}
	
	Rail* rail = train->getCurrentRail();
	if (!rail)
	{
		return 0.0;
	}
	
	return PhysicsSystem::kmhToMs(rail->getSpeedLimit());
}

double CollisionAvoidance::getNextSpeedLimit(const Train* train) const
{
	if (!train)
	{
		return -1.0;
	}
	
	size_t nextIndex = train->getCurrentRailIndex() + 1;
	if (nextIndex >= train->getPath().size())
	{
		return -1.0;
	}
	
	Rail* nextRail = train->getPath()[nextIndex];
	return PhysicsSystem::kmhToMs(nextRail->getSpeedLimit());
}
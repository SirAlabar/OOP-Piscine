#include "simulation/CollisionAvoidance.hpp"
#include "simulation/OccupancyMap.hpp"
#include "simulation/RiskData.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "core/Graph.hpp"
#include "simulation/PhysicsSystem.hpp"
#include <limits>

const OccupancyMap& CollisionAvoidance::getOccupancyMap() const
{
    return _occupancy;
}

void CollisionAvoidance::refreshRailOccupancy(const std::vector<Train*>& trains, const Graph* network)
{
    if (!network)
    {
        return;
    }

    // Clear previous occupancy for all rails.
    _occupancy.clearAll(network->getRails());

    // Rebuild from current train positions.
    for (Train* train : trains)
    {
        if (train && !train->isFinished() && train->getCurrentRail())
        {
            _occupancy.add(train->getCurrentRail(), train);
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
		totalDistance += PhysicsSystem::kmToM(path[i].rail->getLength());
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
        if (path[i].rail == rail)
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

// Check if two trains are traveling in the same direction on a given rail segment
bool CollisionAvoidance::areTravelingSameDirection(const Train* train1, size_t idx1, const Train* train2, size_t idx2) const
{
    const auto& path1 = train1->getPath();
    const auto& path2 = train2->getPath();

    if (idx1 >= path1.size() || idx2 >= path2.size())
    {
        return false;
    }

    // Same direction if both have same fromâ†’to nodes
    return (path1[idx1].from == path2[idx2].from && path1[idx1].to == path2[idx2].to);
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

        // Find if other train's current rail is in my future path
        size_t leaderIndex = 0;
        size_t myIndex = train->getCurrentRailIndex();

        if (!findRailIndexInPath(train, other->getCurrentRail(), myIndex, leaderIndex))
        {
            continue;  // Other train not on my route
        }

        // Check if we're traveling in the same direction
        // If opposite directions, this is NOT a leader/follower situation
        if (!areTravelingSameDirection(train, leaderIndex, other, other->getCurrentRailIndex()))
        {
            continue;
        }

        // Calculate gap only if same direction
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
    double gap = PhysicsSystem::kmToM(myPath[myIndex].rail->getLength()) - train->getPosition();

    // Add the full length of all intermediate rails between the two trains
    for (size_t i = myIndex + 1; i < leaderIndex; ++i)
    {
        gap += PhysicsSystem::kmToM(myPath[i].rail->getLength());
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
	
	Rail* nextRail = train->getPath()[nextIndex].rail;
	return PhysicsSystem::kmhToMs(nextRail->getSpeedLimit());
}
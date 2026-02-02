#include "simulation/CollisionAvoidance.hpp"
#include "simulation/RiskData.hpp"
#include "simulation/SafetyConstants.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "core/Graph.hpp"
#include "simulation/PhysicsSystem.hpp"
#include <limits>

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

Train* CollisionAvoidance::findLeaderOnRoute(const Train* train, const std::vector<Train*>& allTrains) const
{
	if (!train)
	{
		return nullptr;
	}
	
	Rail* myRail = train->getCurrentRail();
	if (!myRail)
	{
		return nullptr;
	}
	
	double myPos = train->getPosition();
	Train* closest = nullptr;
	double minDistance = std::numeric_limits<double>::infinity();
	
	// Check all trains on same rail
	for (Train* other : allTrains)
	{
		if (!other || other == train)
		{
			continue;
		}
		
		if (other->getCurrentRail() == myRail)
		{
			double otherPos = other->getPosition();
			
			// Leader is ahead of me
			if (otherPos > myPos)
			{
				double distance = otherPos - myPos;
				if (distance < minDistance)
				{
					minDistance = distance;
					closest = other;
				}
			}
		}
	}
	
	return closest;
}

double CollisionAvoidance::calculateGap(const Train* train, const Train* leader) const
{
	if (!train || !leader)
	{
		return -1.0;
	}
	
	// Gap on same rail
	if (train->getCurrentRail() == leader->getCurrentRail())
	{
		return leader->getPosition() - train->getPosition();
	}
	
	return -1.0;
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
	
	double speedBasedMargin = train->getVelocity() * SafetyConstants::SAFE_TIME_HEADWAY;
	double minClearance = SafetyConstants::MINIMUM_CLEARANCE;
	
	return minClearance + speedBasedMargin;
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

void CollisionAvoidance::refreshRailOccupancy(const std::vector<Train*>& trains, const Graph* network)
{
	if (!network)
	{
		return;
	}
	
	for (Rail* rail : network->getRails())
	{
		if (rail)
		{
			rail->clearOccupied();
		}
	}
	
	for (Train* train : trains)
	{
		if (!train)
		{
			continue;
		}
		
		Rail* currentRail = train->getCurrentRail();
		if (currentRail)
		{
			currentRail->setOccupiedBy(train);
		}
	}
}
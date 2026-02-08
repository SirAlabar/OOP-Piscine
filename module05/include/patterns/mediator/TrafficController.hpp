#ifndef TRAFFICCONTROLLER_HPP
#define TRAFFICCONTROLLER_HPP

#include <vector>

class Train;
class Rail;
class Graph;
class CollisionAvoidance;

// Mediator pattern: centralized traffic coordination
class TrafficController
{
public:
	enum AccessDecision
	{
		GRANT,
		DENY
	};

	TrafficController(const Graph* network, 
	                  CollisionAvoidance* collisionSystem,
	                  const std::vector<Train*>* allTrains);
	~TrafficController() = default;

	// Core mediator interface: decides who can access which rail
	AccessDecision requestRailAccess(Train* train, Rail* targetRail);

private:
	const Graph* _network;
	CollisionAvoidance* _collisionSystem;
	const std::vector<Train*>* _allTrains;

	// Decision logic helpers
	bool hasSufficientGap(Train* train, Rail* rail) const;
	bool hasHigherPriority(Train* requestingTrain, Train* conflictingTrain) const;
	Train* findConflictingTrain(Train* train, Rail* rail) const;
	double calculateGapToLeader(Train* train, Train* leader, Rail* rail) const;
	bool areTravelingSameDirection(Train* train, Train* other, Rail* rail) const;
};

#endif
#ifndef COLLISIONAVOIDANCE_HPP
#define COLLISIONAVOIDANCE_HPP

#include <vector>

class Train;
class Graph;
class Rail;

class CollisionAvoidance
{
public:
	CollisionAvoidance() = default;
	~CollisionAvoidance() = default;

	void refreshRailOccupancy(const std::vector<Train*>& trains, const Graph* network) const;

	// Safety queries
	bool isNextTrainTooClose(const Train* train, const std::vector<Train*>& trains) const;
	double distanceToNextTrain(const Train* train, const std::vector<Train*>& trains) const;

	bool isNextRailOccupied(const Train* train) const;
    bool shouldWaitForTrainAhead(
    const Train* train,
    const std::vector<Train*>& trains) const;


	// Minimum safe distance rule
	double getMinimumSafeDistance(const Train* train) const;
};

#endif

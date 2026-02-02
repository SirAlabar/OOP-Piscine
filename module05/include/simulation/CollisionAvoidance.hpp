#ifndef COLLISIONAVOIDANCE_HPP
#define COLLISIONAVOIDANCE_HPP

#include <vector>

class Train;
class Graph;
class Rail;
struct RiskData;

class CollisionAvoidance
{
public:
	CollisionAvoidance() = default;
	~CollisionAvoidance() = default;

	void refreshRailOccupancy(const std::vector<Train*>& trains, const Graph* network);
	RiskData assessRisk(const Train* train, const std::vector<Train*>& allTrains) const;
	
private:
	// Internal helpers (all const, no side effects)
	Train* findLeaderOnRoute(const Train* train, const std::vector<Train*>& allTrains) const;
	double calculateGap(const Train* train, const Train* leader) const;
	double calculateClosingSpeed(const Train* train, const Train* leader) const;
	double calculateBrakingDistance(const Train* train) const;
	double calculateSafeDistance(const Train* train) const;
	double getCurrentSpeedLimit(const Train* train) const;
	double getNextSpeedLimit(const Train* train) const;

	// Route-aware helpers
	double calculateAbsoluteRoutePosition(const Train* train) const;
	// bool sharesRoute(const Train* train, const Train* other) const;
    bool findRailIndexInPath(const Train* t, const Rail* rail, size_t startIndex, size_t& outIndex) const;
};

#endif
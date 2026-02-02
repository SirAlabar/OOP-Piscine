#ifndef SIMULATIONCONTEXT_HPP
#define SIMULATIONCONTEXT_HPP

#include <map>
#include <vector>

class Train;
class Graph;
class Node;
class StoppedState;
class CollisionAvoidance;

class SimulationContext
{
private:
	Graph* _network;
	CollisionAvoidance* _collisionSystem;
	const std::vector<Train*>* _trains;
	std::map<Train*, StoppedState*> _stoppedStates;

public:
	SimulationContext(
		Graph* network,
		CollisionAvoidance* collisionSystem,
		const std::vector<Train*>* trains
	);

	~SimulationContext();

	// Physics Queries
	double getCurrentRailSpeedLimit(const Train* train) const;
	double getCurrentRailLength(const Train* train) const;
	double getBrakingDistance(const Train* train) const;
	double getDistanceToRailEnd(const Train* train) const;

	// Network Queries
	Node* getCurrentArrivalNode(const Train* train) const;

	// SAfety Queries
	double distanceToNextTrain(const Train* train) const;
	double getMinimumSafeDistance(const Train* train) const;
    const RiskData& getRisk(const Train* train) const;


	// State Lifecycle
	StoppedState* getOrCreateStoppedState(Train* train, double durationSeconds);
	void releaseStoppedState(Train* train);
};

#endif
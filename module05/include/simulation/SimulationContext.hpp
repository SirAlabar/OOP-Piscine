#ifndef SIMULATIONCONTEXT_HPP
#define SIMULATIONCONTEXT_HPP

#include <map>
#include <vector>

class Train;
class Graph;
class Node;
class CollisionAvoidance;
class TrafficController;
class StateRegistry;
struct RiskData;

class SimulationContext
{
private:
	Graph* _network;
	CollisionAvoidance* _collisionSystem;
	TrafficController* _trafficController;
	const std::vector<Train*>* _trains;
	// State Management
	StateRegistry* _states;
	// Cached Risk Data
	std::map<Train*, RiskData> _riskMap;  // Refreshed once per frame
	std::map<Train*, double> _stopDurations;

public:
	SimulationContext(
		Graph* network,
		CollisionAvoidance* collisionSystem,
		const std::vector<Train*>* trains,
		TrafficController* trafficController
	);
	~SimulationContext();

	// Risk Data Access

	const RiskData& getRisk(const Train* train) const;
	void refreshAllRiskData();

	// Physics Queries
	
	double getCurrentRailSpeedLimit(const Train* train) const;
	double getCurrentRailLength(const Train* train) const;
	double getBrakingDistance(const Train* train) const;
	double getDistanceToRailEnd(const Train* train) const;

	// Network Queries
	
	Node* getCurrentArrivalNode(const Train* train) const;

	// State Registry Access =====

	StateRegistry& states();
	
	// TrafficController Access
	TrafficController* getTrafficController() const;

	// Externalized State Data
	void setStopDuration(Train* train, double durationSeconds);
	double getStopDuration(const Train* train) const;
	bool decrementStopDuration(Train* train, double dt);
	void clearStopDuration(Train* train);

	//Physics Abstraction
	void applyForce(Train* train, double force, double dt);
};

#endif
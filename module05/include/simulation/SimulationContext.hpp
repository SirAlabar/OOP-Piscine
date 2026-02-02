#ifndef SIMULATIONCONTEXT_HPP
#define SIMULATIONCONTEXT_HPP

#include <map>
#include <vector>

class Train;
class Graph;
class Node;
class CollisionAvoidance;
class StateRegistry;
struct RiskData;

class SimulationContext
{
private:
	Graph* _network;
	CollisionAvoidance* _collisionSystem;
	const std::vector<Train*>* _trains;
	// ===== STATE MANAGEMENT =====
	StateRegistry* _states;
	// ===== CACHED RISK DATA =====
	std::map<Train*, RiskData> _riskMap;  // Refreshed once per frame
	std::map<Train*, double> _stopDurations;

public:
	SimulationContext(
		Graph* network,
		CollisionAvoidance* collisionSystem,
		const std::vector<Train*>* trains
	);
	~SimulationContext();

	// ===== RISK DATA ACCESS (READ-ONLY) =====

	const RiskData& getRisk(const Train* train) const;
	void refreshAllRiskData();

	// ===== PHYSICS QUERIES =====
	
	double getCurrentRailSpeedLimit(const Train* train) const;
	double getCurrentRailLength(const Train* train) const;
	double getBrakingDistance(const Train* train) const;
	double getDistanceToRailEnd(const Train* train) const;

	// ===== NETWORK QUERIES =====
	
	Node* getCurrentArrivalNode(const Train* train) const;

	// ===== STATE REGISTRY ACCESS =====

	StateRegistry& states();

	// ===== EXTERNALIZED STATE DATA =====
	void setStopDuration(Train* train, double durationSeconds);
	double getStopDuration(const Train* train) const;
	bool decrementStopDuration(Train* train, double dt);
	void clearStopDuration(Train* train);

	// ===== PHYSICS ABSTRACTION =====

	void applyForce(Train* train, double force, double dt);
};

#endif
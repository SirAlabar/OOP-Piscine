#ifndef RAIL_HPP
#define RAIL_HPP

#include <vector>
#include "patterns/observers/IObserver.hpp"

class Node;
class Train;
class Event;

// Represents a bidirectional rail segment connecting two nodes
class Rail : public IObserver
{
private:
	Node*  _nodeA;
	Node*  _nodeB;
	double _length;      // km
	double _speedLimit;  // km/h
	
	// Multi-train tracking: rails can hold multiple trains with safe spacing
	std::vector<Train*> _trainsOnRail;

public:
	Rail();
	Rail(Node* nodeA, Node* nodeB, double length, double speedLimit);
	Rail(const Rail&) = default;
	Rail& operator=(const Rail&) = default;
	~Rail() = default;

	Node*  getNodeA() const;
	Node*  getNodeB() const;
	double getLength() const;
	double getSpeedLimit() const;
	void   setSpeedLimit(double speedLimit);  // For events (maintenance, weather)
	
	// Multi-train rail occupancy management
	void addTrain(Train* train);
	void removeTrain(Train* train);
	const std::vector<Train*>& getTrainsOnRail() const;
	bool hasTrains() const;
	Train* findLeaderFor(Train* follower, Node* directionTo) const;
	
	bool isValid() const;

	// Get the other node (if current is nodeA, return nodeB and vice versa)
	Node* getOtherNode(Node* current) const;

	// IObserver implementation
	void onNotify(Event* event) override;
};

#endif
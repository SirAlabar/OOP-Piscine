#ifndef RAIL_HPP
#define RAIL_HPP

class Node;
class Train;

// Represents a bidirectional rail segment connecting two nodes
class Rail
{
private:
	Node*  _nodeA;
	Node*  _nodeB;
	double _length;      // km
	double _speedLimit;  // km/h
	Train* _occupiedBy;  // nullptr if free, pointer to train if occupied

public:
	Rail();
	Rail(Node* nodeA, Node* nodeB, double length, double speedLimit);
	Rail(const Rail& other);
	Rail& operator=(const Rail& other);
	~Rail();

	Node*  getNodeA() const;
	Node*  getNodeB() const;
	double getLength() const;
	double getSpeedLimit() const;
	Train* getOccupiedBy() const;

	bool isOccupied() const;
	void setOccupiedBy(Train* train);
	void clearOccupied();
	bool isValid() const;

	// Get the other node (if current is nodeA, return nodeB and vice versa)
	Node* getOtherNode(Node* current) const;
};

#endif
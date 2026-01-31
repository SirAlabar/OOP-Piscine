#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "core/Train.hpp"

// Default constructor
Rail::Rail()
	: _nodeA(nullptr),
	  _nodeB(nullptr),
	  _length(0.0),
	  _speedLimit(0.0),
	  _occupiedBy(nullptr)
{
}

// Parameterized constructor
Rail::Rail(Node* nodeA, Node* nodeB, double length, double speedLimit)
	: _nodeA(nodeA),
	  _nodeB(nodeB),
	  _length(length),
	  _speedLimit(speedLimit),
	  _occupiedBy(nullptr)
{
}

// Getters
Node* Rail::getNodeA() const
{
	return _nodeA;
}

Node* Rail::getNodeB() const
{
	return _nodeB;
}

double Rail::getLength() const
{
	return _length;
}

double Rail::getSpeedLimit() const
{
	return _speedLimit;
}

Train* Rail::getOccupiedBy() const
{
	return _occupiedBy;
}

// Check if rail is currently occupied by a train
bool Rail::isOccupied() const
{
	return _occupiedBy != nullptr;
}

// Mark rail as occupied by a train
void Rail::setOccupiedBy(Train* train)
{
	_occupiedBy = train;
}

// Clear occupancy (train has left the rail)
void Rail::clearOccupied()
{
	_occupiedBy = nullptr;
}

// Validation - both nodes exist, length and speed limit are positive
bool Rail::isValid() const
{
	return _nodeA != nullptr && _nodeB != nullptr &&
	       _length > 0.0 && _speedLimit > 0.0;
}

// Get the opposite node (useful for pathfinding)
Node* Rail::getOtherNode(Node* current) const
{
	if (current == _nodeA)
	{
		return _nodeB;
	}
	if (current == _nodeB)
	{
		return _nodeA;
	}
	return nullptr;
}

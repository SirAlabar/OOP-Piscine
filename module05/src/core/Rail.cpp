#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "core/Train.hpp"

// Default constructor
Rail::Rail()
	: _nodeA(nullptr),
	  _nodeB(nullptr),
	  _length(0.0),
	  _speedLimit(0.0)
{
}

// Parameterized constructor
Rail::Rail(Node* nodeA, Node* nodeB, double length, double speedLimit)
	: _nodeA(nodeA),
	  _nodeB(nodeB),
	  _length(length),
	  _speedLimit(speedLimit)
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

void Rail::setSpeedLimit(double speedLimit)
{
	_speedLimit = speedLimit;
}

// Multi-train rail occupancy management
void Rail::addTrain(Train* train)
{
	if (!train)
	{
		return;
	}

	// Avoid duplicates
	for (Train* t : _trainsOnRail)
	{
		if (t == train)
		{
			return;
		}
	}

	_trainsOnRail.push_back(train);
}

void Rail::removeTrain(Train* train)
{
	if (!train)
	{
		return;
	}

	for (auto it = _trainsOnRail.begin(); it != _trainsOnRail.end(); ++it)
	{
		if (*it == train)
		{
			_trainsOnRail.erase(it);
			return;
		}
	}
}

const std::vector<Train*>& Rail::getTrainsOnRail() const
{
	return _trainsOnRail;
}

bool Rail::hasTrains() const
{
	return !_trainsOnRail.empty();
}

Train* Rail::findLeaderFor(Train* follower, Node* directionTo) const
{
	if (!follower || !directionTo)
	{
		return nullptr;
	}

	Train* nearestLeader = nullptr;
	double minGap = 1e9;

	for (Train* other : _trainsOnRail)
	{
		if (!other || other == follower || other->isFinished())
		{
			continue;
		}

		// Check if other train is traveling in the same direction
		// by comparing the path direction
		const auto& otherPath = other->getPath();
		size_t otherIdx = other->getCurrentRailIndex();
		
		if (otherIdx >= otherPath.size())
		{
			continue;
		}

		// Check if traveling toward the same destination node
		if (otherPath[otherIdx].to != directionTo)
		{
			continue;  // Different direction, not a leader
		}

		// Calculate gap (leader position - follower position)
		double gap = other->getPosition() - follower->getPosition();

		// Only consider trains ahead (positive gap)
		if (gap > 0.0 && gap < minGap)
		{
			minGap = gap;
			nearestLeader = other;
		}
	}

	return nearestLeader;
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
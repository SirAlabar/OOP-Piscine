#include "core/Node.hpp"
#include "core/Rail.hpp"

Rail::Rail()
    : _nodeA(nullptr),
      _nodeB(nullptr),
      _length(0.0),
      _speedLimit(0.0)
{
}

Rail::Rail(Node* nodeA, Node* nodeB, double length, double speedLimit)
    : _nodeA(nodeA),
      _nodeB(nodeB),
      _length(length),
      _speedLimit(speedLimit)
{
}

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

bool Rail::isValid() const
{
    return _nodeA != nullptr
        && _nodeB != nullptr
        && _length > 0.0
        && _speedLimit > 0.0;
}

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
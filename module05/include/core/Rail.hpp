#ifndef RAIL_HPP
#define RAIL_HPP

class Node;

// Represents a bidirectional rail segment connecting two nodes.
// Runtime occupancy (which trains are currently on this rail) has been moved
// to OccupancyMap, owned by CollisionAvoidance.  Rail is now a pure geometric
// data class with no dependency on Train.
class Rail
{
private:
    Node*  _nodeA;
    Node*  _nodeB;
    double _length;      // km
    double _speedLimit;  // km/h

public:
    Rail();
    Rail(Node* nodeA, Node* nodeB, double length, double speedLimit);
    Rail(const Rail&)            = default;
    Rail& operator=(const Rail&) = default;
    ~Rail()                      = default;

    Node*  getNodeA()      const;
    Node*  getNodeB()      const;
    double getLength()     const;
    double getSpeedLimit() const;
    void   setSpeedLimit(double speedLimit);

    bool   isValid()                const;
    Node*  getOtherNode(Node* node) const;
};

#endif
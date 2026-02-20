#ifndef RAIL_HPP
#define RAIL_HPP

#include <vector>

class Node;
class Train;

// Represents a bidirectional rail segment connecting two nodes.
class Rail
{
private:
    Node*  _nodeA;
    Node*  _nodeB;
    double _length;      // km
    double _speedLimit;  // km/h

    // Runtime occupancy — managed by CollisionAvoidance via refreshRailOccupancy().
    std::vector<Train*> _trainsOnRail;

public:
    Rail();
    Rail(Node* nodeA, Node* nodeB, double length, double speedLimit);
    Rail(const Rail&)            = default;
    Rail& operator=(const Rail&) = default;
    ~Rail()                      = default;

    Node*  getNodeA()     const;
    Node*  getNodeB()     const;
    double getLength()    const;
    double getSpeedLimit() const;
    void   setSpeedLimit(double speedLimit);

    // Occupancy management — called exclusively by CollisionAvoidance.
    void                       addTrain(Train* train);
    void                       removeTrain(Train* train);
    const std::vector<Train*>& getTrainsOnRail() const;
    bool                       hasTrains()       const;
    Train*                     findLeaderFor(Train* follower, Node* directionTo) const;

    bool   isValid()                const;
    Node*  getOtherNode(Node* node) const;
};

#endif
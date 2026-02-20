#ifndef IPHYSICSQUERIES_HPP
#define IPHYSICSQUERIES_HPP

class Train;
class Node;

// Read-only physics and network queries that states and movement logic require.
// Implemented by SimulationContext; injectable for unit testing.
class IPhysicsQueries
{
public:
    virtual ~IPhysicsQueries() = default;

    // Speed limit of the rail the train is currently on, in m/s.
    virtual double getCurrentRailSpeedLimit(const Train* train) const = 0;

    // Physical length of the current rail, in metres.
    virtual double getCurrentRailLength(const Train* train) const = 0;

    // Minimum distance required to brake the train to a full stop, in metres.
    virtual double getBrakingDistance(const Train* train) const = 0;

    // Remaining distance to the end of the current rail, in metres.
    virtual double getDistanceToRailEnd(const Train* train) const = 0;

    // Destination node at the far end of the train's current path segment.
    // Returns nullptr when the train has no current rail.
    virtual Node* getCurrentArrivalNode(const Train* train) const = 0;
};

#endif
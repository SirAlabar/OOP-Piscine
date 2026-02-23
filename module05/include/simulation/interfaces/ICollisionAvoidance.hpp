#ifndef ICOLLISIONAVOIDANCE_HPP
#define ICOLLISIONAVOIDANCE_HPP

#include <vector>

class Train;
class Graph;
class Rail;
struct RiskData;
class OccupancyMap;

// Narrow interface for collision detection and occupancy tracking.
// Consumers depend on this; not on the concrete CollisionAvoidance.
class ICollisionAvoidance
{
public:
    virtual ~ICollisionAvoidance() = default;

    virtual void             refreshRailOccupancy(const std::vector<Train*>& trains, const Graph* network) = 0;
    virtual RiskData         assessRisk(const Train* train, const std::vector<Train*>& allTrains) const    = 0;
    virtual const OccupancyMap& getOccupancyMap() const                                                    = 0;
};

#endif
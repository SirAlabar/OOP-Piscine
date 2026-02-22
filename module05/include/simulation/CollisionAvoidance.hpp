#ifndef COLLISIONAVOIDANCE_HPP
#define COLLISIONAVOIDANCE_HPP

#include "simulation/ICollisionAvoidance.hpp"
#include "simulation/OccupancyMap.hpp"
#include <vector>
#include <cstddef>

class Train;
class Graph;
class Rail;
struct RiskData;

class CollisionAvoidance : public ICollisionAvoidance
{
private:
    OccupancyMap _occupancy;

    Train* findLeaderOnRoute(const Train* train, const std::vector<Train*>& allTrains) const;
    double calculateGap(const Train* train, const Train* leader)                       const;
    double calculateClosingSpeed(const Train* train, const Train* leader)               const;
    double calculateBrakingDistance(const Train* train)                                const;
    double calculateSafeDistance(const Train* train)                                   const;
    double getCurrentSpeedLimit(const Train* train)                                    const;
    double getNextSpeedLimit(const Train* train)                                        const;
    double calculateAbsoluteRoutePosition(const Train* train)                          const;

    bool findRailIndexInPath(const Train* t, const Rail* rail,
                             std::size_t startIndex, std::size_t& outIndex)            const;
    bool areTravelingSameDirection(const Train* train1, std::size_t idx1,
                                   const Train* train2, std::size_t idx2)              const;

public:
    CollisionAvoidance()           = default;
    ~CollisionAvoidance() override = default;

    void                refreshRailOccupancy(const std::vector<Train*>& trains, const Graph* network) override;
    RiskData            assessRisk(const Train* train, const std::vector<Train*>& allTrains) const override;
    const OccupancyMap& getOccupancyMap() const override;
};

#endif
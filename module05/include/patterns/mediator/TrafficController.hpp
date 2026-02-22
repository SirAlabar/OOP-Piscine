#ifndef TRAFFICCONTROLLER_HPP
#define TRAFFICCONTROLLER_HPP

#include "patterns/mediator/ITrainController.hpp"
#include <vector>

class Train;
class Rail;
class Graph;
class Node;
class ICollisionAvoidance;

// Mediator: centralized traffic coordination.
class TrafficController : public ITrainController
{
private:
    const Graph*               _network;
    ICollisionAvoidance*       _collisionSystem;
    const std::vector<Train*>* _allTrains;

    bool   hasSufficientGap(Train* train, Rail* rail)                         const;
    bool   hasHigherPriority(Train* requestingTrain, Train* conflictingTrain) const;
    Train* findConflictingTrain(Train* train, Rail* rail)                     const;
    double calculateGapToLeader(Train* train, Train* leader, Rail* rail)      const;
    bool   areTravelingSameDirection(Train* train, Train* other, Rail* rail)  const;

public:
    TrafficController(const Graph*               network,
                      ICollisionAvoidance*       collisionSystem,
                      const std::vector<Train*>* allTrains);
    ~TrafficController() override = default;

    AccessDecision requestRailAccess(Train* train, Rail* targetRail) override;
};

#endif
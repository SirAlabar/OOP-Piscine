#ifndef TRAFFICCONTROLLER_HPP
#define TRAFFICCONTROLLER_HPP

#include "patterns/mediator/ITrainController.hpp"
#include <vector>

class Train;
class Rail;
class Graph;
class CollisionAvoidance;

// Mediator pattern: centralized traffic coordination.
// Implements ITrainController so that dependents can be coded against the
// narrow interface without knowing this concrete class.
class TrafficController : public ITrainController
{
private:
    const Graph*               _network;
    CollisionAvoidance*        _collisionSystem;
    const std::vector<Train*>* _allTrains;

    bool   hasSufficientGap(Train* train, Rail* rail)                            const;
    bool   hasHigherPriority(Train* requestingTrain, Train* conflictingTrain)    const;
    Train* findConflictingTrain(Train* train, Rail* rail)                        const;
    double calculateGapToLeader(Train* train, Train* leader, Rail* rail)         const;
    bool   areTravelingSameDirection(Train* train, Train* other, Rail* rail)     const;

public:
    TrafficController(const Graph*               network,
                      CollisionAvoidance*        collisionSystem,
                      const std::vector<Train*>* allTrains);
    ~TrafficController() override = default;

    // ITrainController
    AccessDecision requestRailAccess(Train* train, Rail* targetRail) override;

};

#endif
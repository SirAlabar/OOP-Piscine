#ifndef SIMULATIONCONTEXT_HPP
#define SIMULATIONCONTEXT_HPP

#include "simulation/IPhysicsQueries.hpp"
#include "simulation/IStopTimerStore.hpp"
#include "patterns/states/StateRegistry.hpp"
#include <map>
#include <vector>

class Train;
class Graph;
class Node;
class CollisionAvoidance;
class ITrainController;
class TrafficController;
struct RiskData;

// Aggregates all per-frame simulation state that train states and movement
// logic need to make decisions.  Implements IPhysicsQueries and IStopTimerStore
// so that collaborators can depend on the narrower interface in tests.
class SimulationContext : public IPhysicsQueries, public IStopTimerStore
{
private:
    Graph*                     _network;
    CollisionAvoidance*        _collisionSystem;
    ITrainController*          _trafficController;
    const std::vector<Train*>* _trains;

    StateRegistry              _states;
    std::map<Train*, RiskData> _riskMap;       // Refreshed once per tick
    std::map<Train*, double>   _stopDurations;

public:
    SimulationContext(
        Graph*                     network,
        CollisionAvoidance*        collisionSystem,
        const std::vector<Train*>* trains,
        ITrainController*          trafficController
    );
    ~SimulationContext() override;

    // Risk data
    // Returns the cached risk assessment for the given train.
    // Precondition: refreshAllRiskData() must have been called this tick and
    // the train must have an active rail.  Asserts in debug if the entry is
    // absent; returns a sentinel default in release builds.
    const RiskData& getRisk(const Train* train) const;

    // Rebuild the entire risk cache from the current collision-avoidance state.
    void refreshAllRiskData();

    // IPhysicsQueries
    double getCurrentRailSpeedLimit(const Train* train) const override;
    double getCurrentRailLength(const Train* train)     const override;
    double getBrakingDistance(const Train* train)       const override;
    double getDistanceToRailEnd(const Train* train)     const override;
    Node*  getCurrentArrivalNode(const Train* train)    const override;

    // IStopTimerStore
    void   setStopDuration(Train* train, double durationSeconds) override;
    double getStopDuration(const Train* train)                   const override;
    bool   decrementStopDuration(Train* train, double dt)        override;
    void   clearStopDuration(Train* train)                       override;

    // State catalog
    StateRegistry&       states();
    const StateRegistry& states() const;

    // Traffic access
    ITrainController* getTrafficController() const;

    // Physics mutation
    // Apply a net force to the train and advance its velocity and position.
    void applyForce(Train* train, double force, double dt);
};

#endif
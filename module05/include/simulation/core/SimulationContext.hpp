#ifndef SIMULATIONCONTEXT_HPP
#define SIMULATIONCONTEXT_HPP

#include "simulation/interfaces/IPhysicsQueries.hpp"
#include "simulation/state/IStopTimerStore.hpp"
#include "patterns/behavioral/states/StateRegistry.hpp"
#include <map>
#include <vector>

class Train;
class Graph;
class Node;
class ICollisionAvoidance;
class ITrainController;
struct RiskData;

class SimulationContext : public IPhysicsQueries, public IStopTimerStore
{
private:
    Graph*                     _network;
    ICollisionAvoidance*       _collisionSystem;
    ITrainController*          _trafficController;
    const std::vector<Train*>* _trains;

    StateRegistry              _states;
    std::map<Train*, RiskData> _riskMap;
    std::map<Train*, double>   _stopDurations;

public:
    SimulationContext(Graph*                     network,
                      ICollisionAvoidance*       collisionSystem,
                      const std::vector<Train*>* trains,
                      ITrainController*          trafficController);
    ~SimulationContext() override;

    // Single canonical active-train predicate used by all services.
    bool isTrainActive(const Train* train) const;
    bool hasAnyActiveTrain()               const;

    const RiskData& getRisk(const Train* train) const;
    void            refreshAllRiskData();

    double getCurrentRailSpeedLimit(const Train* train) const override;
    double getCurrentRailLength(const Train* train)     const override;
    double getBrakingDistance(const Train* train)       const override;
    double getDistanceToRailEnd(const Train* train)     const override;
    Node*  getCurrentArrivalNode(const Train* train)    const override;

    void   setStopDuration(Train* train, double durationSeconds) override;
    double getStopDuration(const Train* train)                   const override;
    bool   decrementStopDuration(Train* train, double dt)        override;
    void   clearStopDuration(Train* train)                       override;

    StateRegistry&       states();
    const StateRegistry& states() const;

    ITrainController* getTrafficController() const;

    void applyForce(Train* train, double force, double dt);
};

#endif
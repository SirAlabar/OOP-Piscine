#ifndef NETWORKSERVICESFACTORY_HPP
#define NETWORKSERVICESFACTORY_HPP

#include <vector>

class Graph;
class Train;
class ICollisionAvoidance;
class TrafficController;
class SimulationContext;
class EventFactory;
class IEventScheduler;
class IRng;

// Returned by NetworkServicesFactory::build(); caller owns all pointers.
struct NetworkServices
{
    TrafficController* trafficController;
    SimulationContext* context;
    EventFactory*      eventFactory;
};

// Centralises all `new` calls for network-bound services.
// SimulationManager calls build() on setNetwork() and buildEventFactory() on setEventSeed().
class NetworkServicesFactory
{
private:
    ICollisionAvoidance*       _collisionSystem;
    const std::vector<Train*>* _trains;

public:
    NetworkServicesFactory(ICollisionAvoidance* collisionSystem, const std::vector<Train*>* trains);

    // Builds all three network-bound services. Caller owns the returned pointers.
    NetworkServices build(Graph* network, IRng& rng, IEventScheduler* eventScheduler) const;

    // Builds only EventFactory. Used when the seed changes but the network is unchanged.
    EventFactory* buildEventFactory(Graph* network, IRng& rng, IEventScheduler* eventScheduler) const;
};

#endif
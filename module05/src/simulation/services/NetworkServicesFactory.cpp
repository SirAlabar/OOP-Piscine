#include "simulation/services/NetworkServicesFactory.hpp"
#include "simulation/interfaces/ICollisionAvoidance.hpp"
#include "simulation/core/SimulationContext.hpp"
#include "patterns/behavioral/mediator/TrafficController.hpp"
#include "patterns/creational/factories/EventFactory.hpp"
#include "core/Graph.hpp"
#include "core/INetworkQuery.hpp"
#include "utils/IRng.hpp"
#include "simulation/interfaces/IEventScheduler.hpp"

NetworkServicesFactory::NetworkServicesFactory(ICollisionAvoidance*       collisionSystem,
                                               const std::vector<Train*>* trains)
    : _collisionSystem(collisionSystem), _trains(trains)
{
}

NetworkServices NetworkServicesFactory::build(Graph* network, IRng& rng, IEventScheduler* eventScheduler) const
{
    NetworkServices services;

    services.trafficController = new TrafficController(network, _collisionSystem, _trains);

    services.context = new SimulationContext(network, _collisionSystem,
                                             _trains, services.trafficController);

    services.eventFactory = new EventFactory(rng, static_cast<const INetworkQuery*>(network), eventScheduler);

    return services;
}

EventFactory* NetworkServicesFactory::buildEventFactory(Graph* network, IRng& rng, IEventScheduler* eventScheduler) const
{
    return new EventFactory(rng, static_cast<const INetworkQuery*>(network), eventScheduler);
}
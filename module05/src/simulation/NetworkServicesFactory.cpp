#include "simulation/NetworkServicesFactory.hpp"
#include "simulation/ICollisionAvoidance.hpp"
#include "simulation/SimulationContext.hpp"
#include "patterns/mediator/TrafficController.hpp"
#include "patterns/factories/EventFactory.hpp"
#include "core/Graph.hpp"
#include "core/INetworkQuery.hpp"
#include "utils/IRng.hpp"
#include "simulation/IEventScheduler.hpp"

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
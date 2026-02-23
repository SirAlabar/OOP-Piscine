#include "patterns/structural/adapter/RailEventAdapter.hpp"
#include "core/Rail.hpp"
#include "events/Event.hpp"

RailEventAdapter::RailEventAdapter(Rail* rail)
    : _rail(rail)
{
}

void RailEventAdapter::onNotify(Event* event)
{
    if (!event || !_rail)
    {
        return;
    }

    // Speed limit modification logic for rail events belongs here.
    // Currently a no-op — TrackMaintenanceEvent and WeatherEvent apply
    // speed changes directly via Rail::setSpeedLimit in their activate().
}

Rail* RailEventAdapter::getRail() const
{
    return _rail;
}
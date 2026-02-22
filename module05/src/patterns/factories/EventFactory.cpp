#include "patterns/factories/EventFactory.hpp"
#include "utils/IRng.hpp"
#include "core/INetworkQuery.hpp"
#include "simulation/IEventScheduler.hpp"
#include "patterns/events/StationDelayEvent.hpp"
#include "patterns/events/TrackMaintenanceEvent.hpp"
#include "patterns/events/SignalFailureEvent.hpp"
#include "patterns/events/WeatherEvent.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include <vector>

const EventConfig EventFactory::CONFIG_STATION_DELAY     = {0.03,  15,  45};
const EventConfig EventFactory::CONFIG_TRACK_MAINTENANCE  = {0.015, 60, 180};
const EventConfig EventFactory::CONFIG_SIGNAL_FAILURE    = {0.01,   5,  20};
const EventConfig EventFactory::CONFIG_WEATHER           = {0.005, 120, 300};

EventFactory::EventFactory(IRng& rng, const INetworkQuery* network, IEventScheduler* eventScheduler)
    : _rng(rng), _network(network), _eventManager(eventScheduler)
{
}

std::vector<Event*> EventFactory::tryGenerateEvents(const Time& currentTime, double timestepSeconds)
{
    (void)timestepSeconds;
    std::vector<Event*> newEvents;

    if (_rng.getBool(CONFIG_STATION_DELAY.probabilityPerTimestep))
    {
        Event* ev = createStationDelay(currentTime);
        if (ev)
        {
            newEvents.push_back(ev);
        }
    }

    if (_rng.getBool(CONFIG_TRACK_MAINTENANCE.probabilityPerTimestep))
    {
        Event* ev = createTrackMaintenance(currentTime);
        if (ev)
        {
            newEvents.push_back(ev);
        }
    }

    if (_rng.getBool(CONFIG_SIGNAL_FAILURE.probabilityPerTimestep))
    {
        Event* ev = createSignalFailure(currentTime);
        if (ev)
        {
            newEvents.push_back(ev);
        }
    }

    if (_rng.getBool(CONFIG_WEATHER.probabilityPerTimestep))
    {
        Event* ev = createWeather(currentTime);
        if (ev)
        {
            newEvents.push_back(ev);
        }
    }

    return newEvents;
}

Event* EventFactory::createStationDelay(const Time& currentTime)
{
    if (!_network)
    {
        return nullptr;
    }

    const auto& nodes = _network->getNodes();
    std::vector<Node*> cityNodes;

    for (Node* node : nodes)
    {
        if (node && node->getType() == NodeType::CITY)
        {
            cityNodes.push_back(node);
        }
    }

    if (cityNodes.empty())
    {
        return nullptr;
    }

    Node* station = cityNodes[_rng.getInt(0, static_cast<int>(cityNodes.size()) - 1)];

    if (!canCreateStationDelay(station))
    {
        return nullptr;
    }

    Time duration = generateDuration(CONFIG_STATION_DELAY);
    int  delayMin = _rng.getInt(CONFIG_STATION_DELAY.minDurationMinutes, CONFIG_STATION_DELAY.maxDurationMinutes);
    Time extraDelay(0, delayMin);

    return new StationDelayEvent(station, currentTime, duration, extraDelay);
}

Event* EventFactory::createTrackMaintenance(const Time& currentTime)
{
    if (!_network)
    {
        return nullptr;
    }

    const auto& rails = _network->getRails();

    if (rails.empty())
    {
        return nullptr;
    }

    Rail* rail = rails[_rng.getInt(0, static_cast<int>(rails.size()) - 1)];

    if (!canCreateTrackMaintenance(rail))
    {
        return nullptr;
    }

    Time   duration       = generateDuration(CONFIG_TRACK_MAINTENANCE);
    double speedReduction = _rng.getDouble(0.4, 0.7);

    return new TrackMaintenanceEvent(rail, currentTime, duration, speedReduction);
}

Event* EventFactory::createSignalFailure(const Time& currentTime)
{
    if (!_network)
    {
        return nullptr;
    }

    const auto& nodes = _network->getNodes();

    if (nodes.empty())
    {
        return nullptr;
    }

    Node* node = nodes[_rng.getInt(0, static_cast<int>(nodes.size()) - 1)];

    if (!canCreateSignalFailure(node))
    {
        return nullptr;
    }

    Time duration = generateDuration(CONFIG_SIGNAL_FAILURE);
    int  stopMin  = _rng.getInt(CONFIG_SIGNAL_FAILURE.minDurationMinutes, CONFIG_SIGNAL_FAILURE.maxDurationMinutes);
    Time stopDur(0, stopMin);

    return new SignalFailureEvent(node, currentTime, duration, stopDur);
}

Event* EventFactory::createWeather(const Time& currentTime)
{
    if (!_network || !canCreateWeather())
    {
        return nullptr;
    }

    const auto& nodes = _network->getNodes();

    if (nodes.empty())
    {
        return nullptr;
    }

    Node*  center       = nodes[_rng.getInt(0, static_cast<int>(nodes.size()) - 1)];
    Time   duration     = generateDuration(CONFIG_WEATHER);
    double radius       = _rng.getDouble(20.0, 50.0);
    double speedReduce  = _rng.getDouble(0.5, 0.8);
    double frictionInc  = _rng.getDouble(0.01, 0.03);

    const char* types[] = {"Heavy Rain", "Storm", "Snow", "Fog"};

    WeatherEvent* event = new WeatherEvent(types[_rng.getInt(0, 3)], center,
                                           currentTime, duration, radius,
                                           speedReduce, frictionInc);

    std::vector<Rail*> affected;

    for (Rail* rail : _network->getRails())
    {
        if (rail && (rail->getNodeA() == center || rail->getNodeB() == center || rail->getLength() <= radius))
        {
            affected.push_back(rail);
        }
    }

    event->setAffectedRails(affected);

    return event;
}

Time EventFactory::generateDuration(const EventConfig& config)
{
    int minutes = _rng.getInt(config.minDurationMinutes, config.maxDurationMinutes);
    return Time(minutes / 60, minutes % 60);
}

bool EventFactory::canCreateStationDelay(Node* node) const
{
    if (!_eventManager)
    {
        return true;
    }

    for (Event* ev : _eventManager->getActiveEvents())
    {
        if (ev->affectsNode(node))
        {
            EventType t = ev->getType();
            if (t == EventType::STATION_DELAY || t == EventType::SIGNAL_FAILURE)
            {
                return false;
            }
        }
    }

    return true;
}

bool EventFactory::canCreateTrackMaintenance(Rail* rail) const
{
    if (!_eventManager)
    {
        return true;
    }

    int count = 0;

    for (Event* ev : _eventManager->getActiveEvents())
    {
        if (ev->getType() == EventType::TRACK_MAINTENANCE)
        {
            ++count;
            if (ev->affectsRail(rail))
            {
                return false;
            }
        }

        if (ev->getType() == EventType::WEATHER && ev->affectsRail(rail))
        {
            return false;
        }
    }

    return count < 3;
}

bool EventFactory::canCreateSignalFailure(Node* node) const
{
    if (!_eventManager)
    {
        return true;
    }

    int count = 0;

    for (Event* ev : _eventManager->getActiveEvents())
    {
        if (ev->getType() == EventType::SIGNAL_FAILURE)
        {
            ++count;
            if (ev->affectsNode(node))
            {
                return false;
            }
        }

        if (ev->getType() == EventType::STATION_DELAY && ev->affectsNode(node))
        {
            return false;
        }
    }

    return count < 2;
}

bool EventFactory::canCreateWeather() const
{
    if (!_eventManager)
    {
        return true;
    }

    for (Event* ev : _eventManager->getActiveEvents())
    {
        if (ev->getType() == EventType::WEATHER)
        {
            return false;
        }
    }

    return true;
}

unsigned int EventFactory::getSeed() const
{
    return _rng.getSeed();
}
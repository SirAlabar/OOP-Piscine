#ifndef EVENTFACTORY_HPP
#define EVENTFACTORY_HPP

#include "patterns/events/Event.hpp"
#include "utils/Time.hpp"
#include <vector>

class IRng;
class INetworkQuery;
class IEventScheduler;
class Node;
class Rail;

struct EventConfig
{
    double probabilityPerTimestep;
    int    minDurationMinutes;
    int    maxDurationMinutes;
};

class EventFactory
{
private:
    IRng&                _rng;          // Non-owning reference — caller owns the RNG.
    const INetworkQuery* _network;      // Non-owning.
    IEventScheduler*     _eventManager; // Non-owning, for conflict checking.

    static const EventConfig CONFIG_STATION_DELAY;
    static const EventConfig CONFIG_TRACK_MAINTENANCE;
    static const EventConfig CONFIG_SIGNAL_FAILURE;
    static const EventConfig CONFIG_WEATHER;

    Event* createStationDelay(const Time& currentTime);
    Event* createTrackMaintenance(const Time& currentTime);
    Event* createSignalFailure(const Time& currentTime);
    Event* createWeather(const Time& currentTime);

    Time generateDuration(const EventConfig& config);

    bool canCreateStationDelay(Node* node)    const;
    bool canCreateTrackMaintenance(Rail* rail) const;
    bool canCreateSignalFailure(Node* node)   const;
    bool canCreateWeather()                   const;

public:
    // rng, network, and eventScheduler are all non-owning; caller manages lifetime.
    EventFactory(IRng& rng, const INetworkQuery* network, IEventScheduler* eventScheduler);

    // Attempt to generate events based on per-minute probability.
    // Returns a (possibly empty) vector of heap-allocated events; caller owns them.
    std::vector<Event*> tryGenerateEvents(const Time& currentTime, double timestepSeconds = 1.0);

    unsigned int getSeed() const;
};

#endif
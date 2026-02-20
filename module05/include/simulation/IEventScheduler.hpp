#ifndef IEVENTSCHEDULER_HPP
#define IEVENTSCHEDULER_HPP

#include <vector>

class Event;
class Node;
class Rail;
class Time;
enum class EventType;

// Narrow interface for scheduling and querying simulation events.
// Implemented by EventManager.
// Injected into any class that needs event data without coupling to the
// full EventManager (ISubject + scheduling + statistics in one god-object).
class IEventScheduler
{
public:
    virtual ~IEventScheduler() = default;

    // Add an event to the scheduled queue.
    virtual void scheduleEvent(Event* event) = 0;

    // Advance the event lifecycle: activate scheduled events whose time has
    // come, expire events that have ended, notify observers.
    virtual void update(const Time& currentTime) = 0;

    // Read-only views of the current state.
    virtual const std::vector<Event*>& getActiveEvents()    const = 0;
    virtual const std::vector<Event*>& getScheduledEvents() const = 0;

    // Convenience queries.
    virtual int  countActiveEventsByType(EventType type)    const = 0;
    virtual bool hasActiveEventAt(Node* node)               const = 0;
    virtual bool hasActiveEventAt(Rail* rail)               const = 0;

    // Lifetime total for statistics.
    virtual int  getTotalEventsGenerated()                  const = 0;

    // Discard all events and reset statistics (called on simulation reset).
    virtual void clear() = 0;
};

#endif
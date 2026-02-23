#ifndef EVENTSCHEDULER_HPP
#define EVENTSCHEDULER_HPP

#include "simulation/interfaces/IEventScheduler.hpp"
#include <vector>

class EventDispatcher;
class Event;
class Node;
class Rail;
class Time;
enum class EventType;

// Single responsibility: manages time-based event lifecycle.
// Activates scheduled events when their time arrives, expires active events
// when they end, and delegates observer notification to EventDispatcher.
// No knowledge of observer lists — calls dispatcher.notify() instead.
class EventScheduler : public IEventScheduler
{
private:
    EventDispatcher&        _dispatcher;
    std::vector<Event*>     _scheduledEvents;
    std::vector<Event*>     _activeEvents;
    int                     _totalEventsGenerated = 0;\
    
public:
    // dispatcher is non-owning; must outlive this object.
    explicit EventScheduler(EventDispatcher& dispatcher);
    ~EventScheduler() override = default;

    // IEventScheduler
    void scheduleEvent(Event* event)                      override;
    void update(const Time& currentTime)                  override;
    const std::vector<Event*>& getActiveEvents()    const override;
    const std::vector<Event*>& getScheduledEvents() const override;
    int  countActiveEventsByType(EventType type)    const override;
    bool hasActiveEventAt(Node* node)               const override;
    bool hasActiveEventAt(Rail* rail)               const override;
    int  getTotalEventsGenerated()                  const override;
    void clear()                                          override;

};

#endif
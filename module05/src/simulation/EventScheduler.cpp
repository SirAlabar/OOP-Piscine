#include "patterns/observers/EventScheduler.hpp"
#include "patterns/observers/EventDispatcher.hpp"
#include "patterns/events/Event.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"

EventScheduler::EventScheduler(EventDispatcher& dispatcher)
    : _dispatcher(dispatcher)
{
}

void EventScheduler::scheduleEvent(Event* event)
{
    if (!event)
    {
        return;
    }

    _scheduledEvents.push_back(event);
    ++_totalEventsGenerated;
}

void EventScheduler::update(const Time& currentTime)
{
    // Activate scheduled events whose time has come.
    auto schedIt = _scheduledEvents.begin();
    while (schedIt != _scheduledEvents.end())
    {
        Event* event = *schedIt;

        if (event->shouldBeActive(currentTime))
        {
            _activeEvents.push_back(event);
            event->update(currentTime);
            _dispatcher.notify(event);
            schedIt = _scheduledEvents.erase(schedIt);
        }
        else
        {
            ++schedIt;
        }
    }

    // Update active events; remove and notify when they expire.
    auto activeIt = _activeEvents.begin();
    while (activeIt != _activeEvents.end())
    {
        Event* event = *activeIt;
        event->update(currentTime);

        if (!event->isActive())
        {
            _dispatcher.notify(event);
            delete event;
            activeIt = _activeEvents.erase(activeIt);
        }
        else
        {
            ++activeIt;
        }
    }
}

const std::vector<Event*>& EventScheduler::getActiveEvents() const
{
    return _activeEvents;
}

const std::vector<Event*>& EventScheduler::getScheduledEvents() const
{
    return _scheduledEvents;
}

int EventScheduler::countActiveEventsByType(EventType type) const
{
    int count = 0;
    for (Event* event : _activeEvents)
    {
        if (event && event->getType() == type)
        {
            ++count;
        }
    }
    return count;
}

bool EventScheduler::hasActiveEventAt(Node* node) const
{
    if (!node)
    {
        return false;
    }

    for (Event* event : _activeEvents)
    {
        if (event && event->affectsNode(node))
        {
            return true;
        }
    }
    return false;
}

bool EventScheduler::hasActiveEventAt(Rail* rail) const
{
    if (!rail)
    {
        return false;
    }

    for (Event* event : _activeEvents)
    {
        if (event && event->affectsRail(rail))
        {
            return true;
        }
    }
    return false;
}

int EventScheduler::getTotalEventsGenerated() const
{
    return _totalEventsGenerated;
}

void EventScheduler::clear()
{
    for (Event* event : _activeEvents)
    {
        delete event;
    }
    _activeEvents.clear();

    for (Event* event : _scheduledEvents)
    {
        delete event;
    }
    _scheduledEvents.clear();

    _totalEventsGenerated = 0;
}
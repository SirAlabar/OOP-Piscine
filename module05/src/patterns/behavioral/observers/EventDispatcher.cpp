#include "event_system/EventDispatcher.hpp"
#include "patterns/behavioral/observers/IObserver.hpp"
#include <algorithm>

void EventDispatcher::attach(IObserver* observer)
{
    if (!observer)
    {
        return;
    }

    for (IObserver* obs : _observers)
    {
        if (obs == observer)
        {
            return;
        }
    }

    _observers.push_back(observer);
}

void EventDispatcher::detach(IObserver* observer)
{
    if (!observer)
    {
        return;
    }

    auto it = std::find(_observers.begin(), _observers.end(), observer);
    if (it != _observers.end())
    {
        _observers.erase(it);
    }
}

void EventDispatcher::notify(Event* event)
{
    if (!event)
    {
        return;
    }

    for (IObserver* observer : _observers)
    {
        if (observer)
        {
            observer->onNotify(event);
        }
    }
}

void EventDispatcher::clearObservers()
{
    _observers.clear();
}
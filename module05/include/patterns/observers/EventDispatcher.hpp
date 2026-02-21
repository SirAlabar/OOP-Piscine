#ifndef EVENTDISPATCHER_HPP
#define EVENTDISPATCHER_HPP

#include "patterns/observers/ISubject.hpp"
#include <vector>

class IObserver;
class Event;

// Single responsibility: manages observer registration and event fan-out.
// No knowledge of time, scheduling, or event lifecycle.
class EventDispatcher : public ISubject
{
private:
    std::vector<IObserver*> _observers;

public:
    EventDispatcher()          = default;
    ~EventDispatcher() override = default;

    // ISubject
    void attach(IObserver* observer) override;
    void detach(IObserver* observer) override;
    void notify(Event* event)        override;

    // Remove all registered observers (called on simulation reset).
    void clearObservers();
};

#endif